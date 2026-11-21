"""Generate Pebble/Rebble Appstore 720x320 banner for Blokk."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parents[1]
store = root / 'store'
font_path = root / 'pebble/resources/fonts/AmericanCaptain.ttf'
hero_path = store / 'emery-03-gradient-blue.png'

W, H = 720, 320
banner = Image.new('RGB', (W, H), (12, 16, 20))

# Sleek dark charcoal with a crisp 45-degree teal corner accent
px = banner.load()
for y in range(H):
    for x in range(W):
        t = (x / (W - 1) * 0.65 + (1 - y / (H - 1)) * 0.35)
        if x + y < 210:
            f = 1.0 - (x + y) / 210.0
            r = int(0 + 10 * f)
            g = int(125 + 45 * f)
            b = int(125 + 45 * f)
        else:
            r = int(12 + 12 * t)
            g = int(16 + 18 * t)
            b = int(22 + 22 * t)
        px[x, y] = (r, g, b)

# Draw giant "BLOKK" title with crisp 45-degree long black drop shadow
title_layer = Image.new('L', (W, H), 0)
td = ImageDraw.Draw(title_layer)
font_title = ImageFont.truetype(str(font_path), 104)
td.text((38, 18), 'BLOKK', font=font_title, fill=255)

tpx = title_layer.load()
for y in range(H):
    for x in range(W):
        if tpx[x, y] > 128:
            for d in range(1, 26):
                sx, sy = x + d, y + d
                if sx < W and sy < H:
                    px[sx, sy] = (0, 0, 0)
for y in range(H):
    for x in range(W):
        if tpx[x, y] > 128:
            px[x, y] = (255, 255, 255)

draw = ImageDraw.Draw(banner)

try:
    font_sub = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 20)
    font_bul = ImageFont.truetype(r'C:\Windows\Fonts\segoeui.ttf', 16)
    font_tag = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 12)
except OSError:
    font_sub = ImageFont.truetype(str(font_path), 24)
    font_bul = ImageFont.truetype(str(font_path), 20)
    font_tag = ImageFont.truetype(str(font_path), 15)

draw.text((40, 120), 'Watchface for Pebble Time & Time 2', font=font_sub, fill=(0, 224, 216))

bullets = [
    '• Crisp 45° long shadow block typography',
    '• 9 color themes + dithered diagonal gradients',
    '• 4 + 4 customizable health & status fields',
    '• Wrist-flick extra stats & on-demand HR'
]
by = 156
for b in bullets:
    draw.text((40, by), b, font=font_bul, fill=(228, 234, 240))
    by += 26

# 9 Blokk color theme swatches
swatches = [
    (255, 0, 0), (255, 85, 0), (170, 170, 0), (0, 170, 0), (0, 170, 170),
    (0, 170, 255), (0, 0, 255), (255, 0, 170), (170, 0, 255)
]
sx = 40
for col in swatches:
    draw.rounded_rectangle([sx, 266, sx + 22, 276], radius=3, fill=col)
    sx += 28

draw.text((40, 288), 'DESIGN: ZOLTAN NAGY  ·  CODE: ANDOR CZAFIK', font=font_tag, fill=(135, 155, 165))

# Hero Pebble Time 2 watch frame on the right
if hero_path.exists():
    im = Image.open(hero_path).convert('RGB')
    cx, cy = 580, 160
    pad = 12
    bx0, by0 = cx - im.width // 2 - pad, cy - im.height // 2 - pad
    bx1, by1 = cx + im.width // 2 + pad, cy + im.height // 2 + pad
    draw.rounded_rectangle([bx0 + 7, by0 + 9, bx1 + 7, by1 + 9], radius=20, fill=(0, 0, 0))
    draw.rounded_rectangle([bx0, by0, bx1, by1], radius=18, fill=(14, 18, 22), outline=(0, 210, 255), width=3)
    banner.paste(im, (cx - im.width // 2, cy - im.height // 2))

out_path = store / 'banner-720x320.png'
banner.save(out_path)
print(f'Saved banner: {out_path}')
