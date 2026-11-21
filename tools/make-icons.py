"""Generate 80x80 and 144x144 Pebble/Rebble Appstore icons for Blokk."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parents[1]
store = root / 'store'
font_path = root / 'pebble/resources/fonts/AmericanCaptain.ttf'


def make_icon(size: int, out_path: Path) -> None:
    # Render at 2x for crisp anti-aliased edges, then downsample cleanly
    S = size * 2
    img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
    px = img.load()

    # Rounded square mask (radius ~18% of size)
    radius = int(S * 0.18)
    mask = Image.new('L', (S, S), 0)
    md = ImageDraw.Draw(mask)
    md.rounded_rectangle([0, 0, S - 1, S - 1], radius=radius, fill=255)
    mpx = mask.load()

    # Fill rounded square with rich Blokk turquoise (#00AAAA -> subtle top-left highlight)
    for y in range(S):
        for x in range(S):
            if mpx[x, y] > 0:
                t = (x / (S - 1) + y / (S - 1)) * 0.5
                r = int(0 + 0 * t)
                g = int(196 - 32 * t)
                b = int(190 - 30 * t)
                px[x, y] = (r, g, b, mpx[x, y])

    # Render giant centered "B" in American Captain
    glyph_layer = Image.new('L', (S, S), 0)
    gd = ImageDraw.Draw(glyph_layer)
    font_size = int(S * 0.78)
    font = ImageFont.truetype(str(font_path), font_size)
    bbox = gd.textbbox((0, 0), 'B', font=font)
    gw, gh = bbox[2] - bbox[0], bbox[3] - bbox[1]
    gx = (S - gw) // 2 - bbox[0] - int(S * 0.02)
    gy = (S - gh) // 2 - bbox[1] - int(S * 0.01)
    gd.text((gx, gy), 'B', font=font, fill=255)

    gpx = glyph_layer.load()

    # Cast 45-degree long black drop shadow inside the rounded square
    shadow = [False] * S
    for y in range(S):
        for x in range(S - 1, -1, -1):
            if gpx[x, y] > 128:
                shadow[x] = True
            elif x > 0 and shadow[x - 1]:
                if mpx[x, y] > 0:
                    px[x, y] = (0, 0, 0, mpx[x, y])
                shadow[x] = True
            else:
                shadow[x] = False

    # Composite crisp white "B" on top
    for y in range(S):
        for x in range(S):
            a = gpx[x, y]
            if a > 0 and mpx[x, y] > 0:
                br, bg, bb, ba = px[x, y]
                fa = a / 255.0
                nr = int(br * (1 - fa) + 255 * fa)
                ng = int(bg * (1 - fa) + 255 * fa)
                nb = int(bb * (1 - fa) + 255 * fa)
                px[x, y] = (nr, ng, nb, mpx[x, y])

    final = img.resize((size, size), Image.Resampling.LANCZOS)
    final.save(out_path)
    print(f'Saved {out_path} ({size}x{size})')


make_icon(80, store / 'icon-80x80.png')
make_icon(144, store / 'icon-144x144.png')
