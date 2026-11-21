#include <pebble.h>
#include <stdint.h>
#include <stdlib.h>

#define SCREEN_W PBL_DISPLAY_WIDTH
#define SCREEN_H PBL_DISPLAY_HEIGHT

#if defined(PBL_PLATFORM_BASALT)
  #define HAS_HR_SENSOR 0
  #define RES_FONT_TIME  RESOURCE_ID_FONT_TIME_60
  #define RES_FONT_DATE  RESOURCE_ID_FONT_DATE_16
  #define RES_FONT_WDAY  RESOURCE_ID_FONT_WDAY_11
  #define RES_FONT_INFO  RESOURCE_ID_FONT_INFO_16
  #define RES_FONT_LABEL RESOURCE_ID_FONT_LABEL_12
  #define RES_FONT_AMPM  RESOURCE_ID_FONT_AMPM_18
  #define TIME_Y         34
  #define TIME_BOX_H     68
  #define DIGIT_GAP      3
  #define COLON_W        10
  #define COLON_R        4
  #define COLON_Y1_OFF   27
  #define COLON_Y2_OFF   48
  #define AMPM_RECT      GRect(8, 128, 32, 24)
  #define DATE_Y         14
  #define DATE_GAP       3
  #define WDAY_Y_OFF     1
  #define INFO_SPLIT_X   { 59, 83, 114, 104 }
  #define INFO_ROW_Y     { 99, 120, 99, 141 }
  #define LABEL_X_OFF    26
  #define LABEL_W        24
  #define LABEL_Y_OFF    4
  #define VAL_W          40
#else
  #define HAS_HR_SENSOR 1
  #define RES_FONT_TIME  RESOURCE_ID_FONT_TIME_84
  #define RES_FONT_DATE  RESOURCE_ID_FONT_DATE_22
  #define RES_FONT_WDAY  RESOURCE_ID_FONT_WDAY_14
  #define RES_FONT_INFO  RESOURCE_ID_FONT_INFO_22
  #define RES_FONT_LABEL RESOURCE_ID_FONT_LABEL_15
  #define RES_FONT_AMPM  RESOURCE_ID_FONT_AMPM_24
  #define TIME_Y         48
  #define TIME_BOX_H     92
  #define DIGIT_GAP      5
  #define COLON_W        14
  #define COLON_R        6
  #define COLON_Y1_OFF   37
  #define COLON_Y2_OFF   66
  #define AMPM_RECT      GRect(12, 174, 40, 30)
  #define DATE_Y         22
  #define DATE_GAP       4
  #define WDAY_Y_OFF     2
  #define INFO_SPLIT_X   { 82, 115, 158, 144 }
  #define INFO_ROW_Y     { 138, 166, 138, 194 }
  #define LABEL_X_OFF    34
  #define LABEL_W        32
  #define LABEL_Y_OFF    6
  #define VAL_W          54
#endif

#define SETTINGS_KEY 1
#define COLOR_STATE_KEY 2
#define HR_SHAKE_KEY 3
#define SETTINGS_VERSION 1
#define SECONDARY_TIMEOUT_MS 20000
#define HR_SHAKE_DURATION_MS 45000
#define HR_CONFIRMATION_TOLERANCE_BPM 5
#define HR_SHAKE_RESULT_HOLD_SEC 60

typedef enum {
  BG_OPTION_FIX = 0,
  BG_OPTION_MINUTE = 1,
  BG_OPTION_HOUR = 2,
  BG_OPTION_SHAKE = 3,
  BG_OPTION_DAY = 4,
  BG_OPTION_TIME_OF_DAY = 5
} BgOption;

typedef enum {
  INFO_NONE = 0,
  INFO_STEPS = 1,
  INFO_BPM = 2,
  INFO_BATTERY = 3,
  INFO_DISTANCE = 4,
  INFO_SLEEP = 5,
  INFO_CALORIES = 6,
  INFO_ACTIVE = 7,
  INFO_BATTERY_LOW = 8,
  INFO_SECONDS = 9
} InfoType;

typedef struct {
  int version;
  int bg_option;
  int bg_color;
  int bg_gradient;
  int fields[8];
  int hour_mode;      // 0=watch, 1=24h, 2=12h
  int show_ampm;      // 0/1
  int dist_unit;      // 0=default(metric), 1=metric, 2=US
  int secondary_info; // 0/1
  int tap_vibe;       // 0/1
  int vibe_in_quiet;  // 0/1
} Settings;

typedef struct {
  int active_color;
  int last_day;
  int last_hour;
  int last_minute;
} ColorState;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Rgb;

// Exact Fitbit Blokk hour->color schedule (0..23)
static const uint8_t COLORS_HOURS[24] = {
  8, 8, 8, 0, 0, 0, 1, 1, 1, 2, 2, 2,
  4, 4, 4, 4, 5, 5, 5, 5, 8, 8, 8, 8
};

// Solid Pebble 64-color palette matching Fitbit Blokk's 9 themes (tuned for e-paper contrast)
static const uint8_t SOLID_COLORS[9] = {
  GColorRedARGB8,              // 0: Red (#ff0000)
  GColorOrangeARGB8,           // 1: Orange (#ff5500)
  GColorLimerickARGB8,         // 2: Yellow (#aaaa00)
  GColorIslamicGreenARGB8,     // 3: Green (#00aa00)
  GColorTiffanyBlueARGB8,      // 4: Turquoise (#00aaaa)
  GColorVividCeruleanARGB8,    // 5: Blue (#00aaff)
  GColorBlueARGB8,             // 6: Dark Blue (#0000ff)
  GColorFashionMagentaARGB8,   // 7: Pink (#ff00aa)
  GColorVividVioletARGB8       // 8: Purple (#aa00ff)
};

// Diagonal BL -> TR gradient endpoints sampled from original bg0..bg8.png
static const Rgb GRAD_BL[9] = {
  {230, 180,  20}, // 0: Red -> warm yellow-orange
  {225, 245,   5}, // 1: Orange -> bright yellow
  { 24, 254, 204}, // 2: Yellow -> mint/aqua
  {231, 241,  22}, // 3: Green -> yellow-green
  {210, 245, 225}, // 4: Turquoise -> pale mint
  {168,  96, 252}, // 5: Blue -> violet-purple
  { 18, 187, 255}, // 6: Dark Blue -> sky blue
  {164, 128, 255}, // 7: Pink -> lavender
  {245, 170, 233}  // 8: Purple -> soft pink
};

static const Rgb GRAD_TR[9] = {
  {255,   0,   0}, // 0: Red
  {255, 130,   0}, // 1: Orange
  {230, 215,   0}, // 2: Yellow
  {100, 251,   0}, // 3: Green
  { 12, 226, 188}, // 4: Turquoise
  {  1, 163, 248}, // 5: Blue
  {  0,  23, 254}, // 6: Dark Blue
  {228,   2, 215}, // 7: Pink
  {158,   0, 251}  // 8: Purple
};

static const uint8_t BAYER4[4][4] = {
  { 0,  8,  2, 10},
  {12,  4, 14,  6},
  { 3, 11,  1,  9},
  {15,  7, 13,  5}
};

static const char *MONTH_NAMES[12] = {
  "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
  "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"
};

static const char *WDAY_NAMES[7] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

static Window *s_window;
static Layer *s_canvas;
static GFont s_font_time;
static GFont s_font_date;
static GFont s_font_wday;
static GFont s_font_info;
static GFont s_font_label;
static GFont s_font_ampm;

static Settings s_settings = {
  .version = SETTINGS_VERSION,
  .bg_option = BG_OPTION_TIME_OF_DAY,
  .bg_color = 2,
  .bg_gradient = 0,
#if defined(PBL_PLATFORM_BASALT)
  .fields = {
    INFO_STEPS, INFO_SECONDS, INFO_ACTIVE, INFO_BATTERY,
    INFO_CALORIES, INFO_NONE, INFO_SLEEP, INFO_DISTANCE
  },
#else
  .fields = {
    INFO_STEPS, INFO_BPM, INFO_ACTIVE, INFO_BATTERY,
    INFO_CALORIES, INFO_SECONDS, INFO_SLEEP, INFO_DISTANCE
  },
#endif
  .hour_mode = 0,
  .show_ampm = 0,
  .dist_unit = 0,
  .secondary_info = 0,
  .tap_vibe = 1,
  .vibe_in_quiet = 0
};

static ColorState s_color_state = {
  .active_color = 2,
  .last_day = -1,
  .last_hour = -1,
  .last_minute = -1
};

static struct tm s_now;
static bool s_second_fields = false;
static bool s_seconds_subscribed = false;
static bool s_health_subscribed = false;
static AppTimer *s_secondary_timer = NULL;

static bool s_hr_on_shake = false;
static AppTimer *s_hr_measurement = NULL;
static AppTimer *s_hr_blink_timer = NULL;
static bool s_hr_dim = false;
#if HAS_HR_SENSOR
static HealthValue s_hr_raw_at_start = 0;
static HealthValue s_hr_candidate = 0;
static time_t s_hr_measured_at = 0;
#endif

static int s_battery = 100;
static int s_steps = -1;
static int s_hr = -1;
static int s_distance_m = -1;
static int s_calories = -1;
static int s_active_min = -1;
static int s_sleep_sec = -1;

static uint8_t quantize_dither(int val, int threshold) {
  int scaled = val * 3;
  int q = scaled / 255;
  int rem = scaled % 255;
  if (q < 3 && rem * 16 > threshold * 255) {
    q++;
  }
  return (uint8_t)q;
}

static uint8_t bg_pixel_color(int x, int y, int color_id, bool gradient) {
  if (color_id < 0 || color_id > 8) color_id = 2;
  if (!gradient) {
    return SOLID_COLORS[color_id];
  }
  // 45-degree diagonal interpolation from bottom-left (t=0) to top-right (t=255)
  int tx = (x * 255) / (SCREEN_W - 1);
  int ty = ((SCREEN_H - 1 - y) * 255) / (SCREEN_H - 1);
  int t = (tx + ty) >> 1;

  const Rgb *c0 = &GRAD_BL[color_id];
  const Rgb *c1 = &GRAD_TR[color_id];
  int r = c0->r + (((int)c1->r - (int)c0->r) * t) / 255;
  int g = c0->g + (((int)c1->g - (int)c0->g) * t) / 255;
  int b = c0->b + (((int)c1->b - (int)c0->b) * t) / 255;

  int thr = BAYER4[y & 3][x & 3];
  uint8_t r2 = quantize_dither(r, thr);
  uint8_t g2 = quantize_dither(g, thr);
  uint8_t b2 = quantize_dither(b, thr);
  return (uint8_t)(0xC0 | (r2 << 4) | (g2 << 2) | b2);
}

static int random_color_except(int current) {
  int next = rand() % 9;
  if (next == current) {
    next = (next + 1 + (rand() % 8)) % 9;
  }
  return next;
}

static int resolve_bg_color(bool from_shake) {
  bool changed = false;
  switch (s_settings.bg_option) {
    case BG_OPTION_FIX:
      if (s_color_state.active_color != s_settings.bg_color) {
        s_color_state.active_color = s_settings.bg_color;
        changed = true;
      }
      break;
    case BG_OPTION_TIME_OF_DAY: {
      int h = s_now.tm_hour;
      if (h < 0 || h > 23) h = 0;
      s_color_state.active_color = COLORS_HOURS[h];
      break;
    }
    case BG_OPTION_DAY:
      if (s_now.tm_mday != s_color_state.last_day) {
        s_color_state.last_day = s_now.tm_mday;
        s_color_state.active_color = random_color_except(s_color_state.active_color);
        changed = true;
      }
      break;
    case BG_OPTION_HOUR:
      if (s_now.tm_hour != s_color_state.last_hour) {
        s_color_state.last_hour = s_now.tm_hour;
        s_color_state.active_color = random_color_except(s_color_state.active_color);
        changed = true;
      }
      break;
    case BG_OPTION_MINUTE:
      if (s_now.tm_min != s_color_state.last_minute) {
        s_color_state.last_minute = s_now.tm_min;
        s_color_state.active_color = random_color_except(s_color_state.active_color);
        changed = true;
      }
      break;
    case BG_OPTION_SHAKE:
      if (from_shake) {
        s_color_state.active_color = random_color_except(s_color_state.active_color);
        changed = true;
      }
      break;
  }
  if (s_color_state.active_color < 0 || s_color_state.active_color > 8) {
    s_color_state.active_color = 2;
  }
  if (changed && s_settings.bg_option != BG_OPTION_MINUTE) {
    persist_write_data(COLOR_STATE_KEY, &s_color_state, sizeof(s_color_state));
  }
  return s_color_state.active_color;
}

static bool is_24h_mode(void) {
  if (s_settings.hour_mode == 1) return true;
  if (s_settings.hour_mode == 2) return false;
  return clock_is_24h_style();
}

static bool is_seconds_needed(void) {
  int base = s_second_fields ? 4 : 0;
  for (int i = 0; i < 4; i++) {
    if (s_settings.fields[base + i] == INFO_SECONDS) return true;
  }
  return false;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed);

static void update_tick_subscription(void) {
  bool need_sec = is_seconds_needed();
  if (need_sec != s_seconds_subscribed) {
    tick_timer_service_unsubscribe();
    tick_timer_service_subscribe(need_sec ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
    s_seconds_subscribed = need_sec;
  }
}

static void restore_system_hr(void) {
#if defined(PBL_HEALTH) && HAS_HR_SENSOR
  if (health_service_set_heart_rate_sample_period(0)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "HR sampling returned to system default");
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Could not clear HR sampling request");
  }
#endif
}

static void stop_hr_blink(void) {
  if (s_hr_blink_timer) {
    app_timer_cancel(s_hr_blink_timer);
    s_hr_blink_timer = NULL;
  }
  s_hr_dim = false;
}

#if HAS_HR_SENSOR
static void blink_hr_label(void *context) {
  s_hr_blink_timer = NULL;
  if (!s_hr_measurement) return;
  s_hr_dim = !s_hr_dim;
  layer_mark_dirty(s_canvas);
  s_hr_blink_timer = app_timer_register(500, blink_hr_label, NULL);
  if (!s_hr_blink_timer) s_hr_dim = false;
}
#endif

static void cancel_hr_measurement(void) {
  if (!s_hr_measurement) return;
  app_timer_cancel(s_hr_measurement);
  s_hr_measurement = NULL;
  stop_hr_blink();
  restore_system_hr();
}

static void refresh_heart_rate(void) {
#if defined(PBL_HEALTH) && HAS_HR_SENSOR
  time_t now = time(NULL);
  if (s_hr_measurement || (s_hr_measured_at && (now - s_hr_measured_at) < HR_SHAKE_RESULT_HOLD_SEC)) {
    return;
  }
  const bool hr_available =
    (health_service_metric_accessible(HealthMetricHeartRateBPM, now, now) & HealthServiceAccessibilityMaskAvailable) != 0;
  HealthValue filtered = hr_available ? health_service_peek_current_value(HealthMetricHeartRateBPM) : 0;
  s_hr = (filtered > 0) ? (int)filtered : -1;
#else
  s_hr = -1;
#endif
}

#if HAS_HR_SENSOR
static void hr_measurement_ended(void *context) {
  s_hr_measurement = NULL;
  stop_hr_blink();
  restore_system_hr();
  refresh_heart_rate();
  APP_LOG(APP_LOG_LEVEL_INFO, "Shake HR measurement timed out");
  layer_mark_dirty(s_canvas);
}
#endif

static bool start_hr_measurement(void) {
#if defined(PBL_HEALTH) && HAS_HR_SENSOR
  if (!s_hr_on_shake || s_hr_measurement || !s_health_subscribed) return false;
  s_hr_raw_at_start = health_service_peek_current_value(HealthMetricHeartRateRawBPM);
  s_hr_candidate = 0;
  if (!health_service_set_heart_rate_sample_period(1)) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Shake HR sampling request declined");
    return false;
  }
  s_hr_measurement = app_timer_register(HR_SHAKE_DURATION_MS, hr_measurement_ended, NULL);
  if (!s_hr_measurement) {
    restore_system_hr();
    return false;
  }
  blink_hr_label(NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Shake HR measurement started");
  return true;
#else
  return false;
#endif
}

static void refresh_health(void) {
#if defined(PBL_HEALTH)
  HealthServiceAccessibilityMask mask;

  mask = health_service_metric_accessible(HealthMetricStepCount, time_start_of_today(), time(NULL));
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_steps = (int)health_service_sum_today(HealthMetricStepCount);
  }

  mask = health_service_metric_accessible(HealthMetricWalkedDistanceMeters, time_start_of_today(), time(NULL));
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_distance_m = (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
  }

  mask = health_service_metric_accessible(HealthMetricActiveSeconds, time_start_of_today(), time(NULL));
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_active_min = (int)(health_service_sum_today(HealthMetricActiveSeconds) / 60);
  }

  mask = health_service_metric_accessible(HealthMetricActiveKCalories, time_start_of_today(), time(NULL));
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    int act = (int)health_service_sum_today(HealthMetricActiveKCalories);
    int rest = (int)health_service_sum_today(HealthMetricRestingKCalories);
    s_calories = act + rest;
  }

  mask = health_service_metric_accessible(HealthMetricSleepSeconds, time_start_of_today(), time(NULL));
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_sleep_sec = (int)health_service_sum_today(HealthMetricSleepSeconds);
  }

  refresh_heart_rate();
#endif
}

static void format_info_field(int info_type, char *label, size_t label_sz, char *val, size_t val_sz, bool *hide) {
  *hide = false;
  label[0] = '\0';
  val[0] = '\0';

  switch (info_type) {
    case INFO_NONE:
      *hide = true;
      break;
    case INFO_STEPS:
      snprintf(label, label_sz, "STP");
      if (s_steps < 0) snprintf(val, val_sz, "--");
      else if (s_steps > 99999) snprintf(val, val_sz, "99999");
      else snprintf(val, val_sz, "%04d", s_steps);
      break;
    case INFO_BPM:
      snprintf(label, label_sz, "BPM");
      if (s_hr <= 0) snprintf(val, val_sz, "--");
      else snprintf(val, val_sz, "%03d", s_hr);
      break;
    case INFO_BATTERY_LOW:
      if (s_battery > 30) {
        *hide = true;
        break;
      }
      // fallthrough
    case INFO_BATTERY:
      snprintf(label, label_sz, "BTT");
      snprintf(val, val_sz, "%03d", s_battery);
      break;
    case INFO_DISTANCE: {
      snprintf(label, label_sz, "DST");
      if (s_distance_m < 0) {
        snprintf(val, val_sz, "--");
      } else {
        // Compute in hundredths of km or miles
        int hundredths;
        if (s_settings.dist_unit == 2) {
          // miles * 100 = meters * 100 / 1609.344
          hundredths = (int)(((int64_t)s_distance_m * 1000 + 8046) / 16093);
        } else {
          // km * 100 = meters / 10
          hundredths = (s_distance_m + 5) / 10;
        }
        int whole = hundredths / 100;
        int frac = hundredths % 100;
        if (whole >= 100) {
          snprintf(val, val_sz, "%d", whole);
        } else if (whole >= 10) {
          snprintf(val, val_sz, "%d.%d", whole, frac / 10);
        } else {
          snprintf(val, val_sz, "%d.%02d", whole, frac);
        }
      }
      break;
    }
    case INFO_SLEEP: {
      snprintf(label, label_sz, "SLP");
      if (s_sleep_sec < 0) {
        snprintf(val, val_sz, "--");
      } else {
        int tenths = (s_sleep_sec + 180) / 360; // hours * 10
        snprintf(val, val_sz, "%d.%d", tenths / 10, tenths % 10);
      }
      break;
    }
    case INFO_CALORIES:
      snprintf(label, label_sz, "CAL");
      if (s_calories < 0) snprintf(val, val_sz, "--");
      else if (s_calories > 9999) snprintf(val, val_sz, "9999");
      else snprintf(val, val_sz, "%04d", s_calories);
      break;
    case INFO_ACTIVE:
      snprintf(label, label_sz, "ACT");
      if (s_active_min < 0) snprintf(val, val_sz, "--");
      else if (s_active_min > 999) snprintf(val, val_sz, "999");
      else snprintf(val, val_sz, "%03d", s_active_min);
      break;
    case INFO_SECONDS:
      snprintf(label, label_sz, "SEC");
      snprintf(val, val_sz, "%02d", s_now.tm_sec);
      break;
    default:
      *hide = true;
      break;
  }
}

static int measure_width(const char *str, GFont font) {
  GSize sz = graphics_text_layout_get_content_size(
    str, font, GRect(0, 0, SCREEN_W, 100), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft
  );
  return sz.w;
}

static void draw_time_and_shadow(GContext *ctx) {
  // 1. Clear to pure black so white glyphs stand out cleanly
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, SCREEN_H), 0, GCornerNone);

  // 2. Prepare digits
  bool is_24 = is_24h_mode();
  int hour = s_now.tm_hour;
  bool is_pm = (hour >= 12);
  if (!is_24) {
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  int min = s_now.tm_min;

  char d[4][2] = {
    { (char)('0' + (hour / 10)), '\0' },
    { (char)('0' + (hour % 10)), '\0' },
    { (char)('0' + (min / 10)),  '\0' },
    { (char)('0' + (min % 10)),  '\0' }
  };

  int w[4];
  for (int i = 0; i < 4; i++) {
    w[i] = measure_width(d[i], s_font_time);
  }

  const int digit_gap = DIGIT_GAP;
  const int colon_w = COLON_W;
  const int colon_r = COLON_R;
  const int total_w = w[0] + digit_gap + w[1] + digit_gap + colon_w + digit_gap + w[2] + digit_gap + w[3];
  int x = (SCREEN_W - total_w) / 2;
  const int time_y = TIME_Y;

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);

  // Draw h1
  graphics_draw_text(ctx, d[0], s_font_time, GRect(x, time_y, w[0] + 6, TIME_BOX_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  x += w[0] + digit_gap;

  // Draw h2
  graphics_draw_text(ctx, d[1], s_font_time, GRect(x, time_y, w[1] + 6, TIME_BOX_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  x += w[1] + digit_gap;

  // Draw colon as two crisp solid white circles (matching original dot.png)
  int colon_cx = x + colon_w / 2;
  graphics_fill_circle(ctx, GPoint(colon_cx, time_y + COLON_Y1_OFF), colon_r);
  graphics_fill_circle(ctx, GPoint(colon_cx, time_y + COLON_Y2_OFF), colon_r);
  x += colon_w + digit_gap;

  // Draw m1
  graphics_draw_text(ctx, d[2], s_font_time, GRect(x, time_y, w[2] + 6, TIME_BOX_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  x += w[2] + digit_gap;

  // Draw m2
  graphics_draw_text(ctx, d[3], s_font_time, GRect(x, time_y, w[3] + 6, TIME_BOX_H),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Optional AM / PM indicator in bottom-left (casts its own 45-degree black shadow!)
  if (!is_24 && s_settings.show_ampm) {
    const char *ampm = is_pm ? "PM" : "AM";
    graphics_draw_text(ctx, ampm, s_font_ampm, AMPM_RECT,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  // 3. Single-pass 45-degree long drop shadow + colorful background fill
  int color_id = resolve_bg_color(false);
  bool gradient = (s_settings.bg_gradient != 0);

  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  if (fb) {
    uint8_t shadow[SCREEN_W];
    memset(shadow, 0, sizeof(shadow));

    for (int py = 0; py < SCREEN_H; py++) {
      GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, py);
      uint8_t *row = info.data;
      int max_x = info.max_x < SCREEN_W ? info.max_x : (SCREEN_W - 1);
      int min_x = info.min_x;

      for (int px = max_x; px >= min_x; px--) {
        if (row[px] == GColorWhiteARGB8) {
          shadow[px] = 1;
        } else if (px > 0 && shadow[px - 1]) {
          row[px] = GColorBlackARGB8;
          shadow[px] = 1;
        } else {
          row[px] = bg_pixel_color(px, py, color_id, gradient);
          shadow[px] = 0;
        }
      }
    }
    graphics_release_frame_buffer(ctx, fb);
  }
}

static void draw_date(GContext *ctx) {
  int mon = s_now.tm_mon;
  if (mon < 0 || mon > 11) mon = 0;
  int wday = s_now.tm_wday;
  if (wday < 0 || wday > 6) wday = 0;

  char main_date[20];
  snprintf(main_date, sizeof(main_date), "%s %02d", MONTH_NAMES[mon], s_now.tm_mday);
  const char *wd = WDAY_NAMES[wday];

  int w_main = measure_width(main_date, s_font_date);
  int w_wday = measure_width(wd, s_font_wday);
  const int gap = DATE_GAP;
  int total_w = w_main + gap + w_wday;
  int start_x = (SCREEN_W - total_w) / 2;
  const int date_y = DATE_Y;

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, main_date, s_font_date, GRect(start_x, date_y, w_main + 6, 28),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, wd, s_font_wday, GRect(start_x + w_main + gap, date_y + WDAY_Y_OFF, w_wday + 6, 20),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void draw_info_fields(GContext *ctx) {
  static const int split_x[4] = INFO_SPLIT_X;
  static const int row_y[4]   = INFO_ROW_Y;

  int base = s_second_fields ? 4 : 0;

  for (int i = 0; i < 4; i++) {
    int info_type = s_settings.fields[base + i];
    char label[8];
    char val[12];
    bool hide = false;
    format_info_field(info_type, label, sizeof(label), val, sizeof(val), &hide);
    if (hide) continue;

    // During on-demand HR measurement, blink the BPM label between dim and normal white
    GColor label_color = (info_type == INFO_BPM && s_hr_measurement && s_hr_dim)
      ? GColorDarkGray
      : GColorWhite;
    graphics_context_set_text_color(ctx, label_color);
    graphics_draw_text(ctx, label, s_font_label, GRect(split_x[i] - LABEL_X_OFF, row_y[i] + LABEL_Y_OFF, LABEL_W, 20),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

    // Value left-aligned just right of split_x
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, val, s_font_info, GRect(split_x[i] + 1, row_y[i], VAL_W, 28),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  draw_time_and_shadow(ctx);
  draw_date(ctx);
  draw_info_fields(ctx);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_now = *tick_time;
  if (units_changed & MINUTE_UNIT) {
    refresh_health();
  }
  layer_mark_dirty(s_canvas);
}

static void battery_handler(BatteryChargeState state) {
  s_battery = state.charge_percent;
  layer_mark_dirty(s_canvas);
}

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
#if HAS_HR_SENSOR
  if (event == HealthEventHeartRateUpdate && s_hr_measurement) {
    const HealthValue raw = health_service_peek_current_value(HealthMetricHeartRateRawBPM);
    if (raw <= 0 || raw == s_hr_raw_at_start) {
      s_hr_candidate = 0;
    } else if (s_hr_candidate > 0 && abs(raw - s_hr_candidate) <= HR_CONFIRMATION_TOLERANCE_BPM) {
      s_hr = (int)raw;
      s_hr_measured_at = time(NULL);
      cancel_hr_measurement();
      APP_LOG(APP_LOG_LEVEL_INFO, "Shake HR measurement received a fresh result: %d", s_hr);
    } else {
      s_hr_candidate = raw;
    }
  }
#endif
  refresh_health();
  layer_mark_dirty(s_canvas);
}
#endif

static void secondary_timeout_cb(void *context) {
  s_secondary_timer = NULL;
  if (s_second_fields) {
    s_second_fields = false;
    update_tick_subscription();
    layer_mark_dirty(s_canvas);
  }
}

static void vibe_tap(void) {
  if (!s_settings.tap_vibe) return;
  if (quiet_time_is_active() && !s_settings.vibe_in_quiet) return;
  static const uint32_t segments[] = { 60 };
  VibePattern pat = { .durations = segments, .num_segments = 1 };
  vibes_enqueue_custom_pattern(pat);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  bool did_action = false;

  if (s_settings.bg_option == BG_OPTION_SHAKE) {
    resolve_bg_color(true);
    did_action = true;
  }

  if (s_settings.secondary_info) {
    s_second_fields = !s_second_fields;
    if (s_secondary_timer) {
      app_timer_cancel(s_secondary_timer);
      s_secondary_timer = NULL;
    }
    if (s_second_fields) {
      s_secondary_timer = app_timer_register(SECONDARY_TIMEOUT_MS, secondary_timeout_cb, NULL);
    }
    update_tick_subscription();
    did_action = true;
  }

  if (start_hr_measurement()) {
    did_action = true;
  }

  if (did_action) {
    vibe_tap();
    layer_mark_dirty(s_canvas);
  }
}

static bool read_int_tuple(DictionaryIterator *iter, uint32_t key, int *dest) {
  Tuple *t = dict_find(iter, key);
  if (!t) return false;
  int v = (int)t->value->int32;
  if (*dest != v) {
    *dest = v;
    return true;
  }
  return false;
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool changed = false;

  changed |= read_int_tuple(iter, MESSAGE_KEY_BG_OPTION, &s_settings.bg_option);
  changed |= read_int_tuple(iter, MESSAGE_KEY_BG_COLOR, &s_settings.bg_color);
  changed |= read_int_tuple(iter, MESSAGE_KEY_BG_GRADIENT, &s_settings.bg_gradient);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD1, &s_settings.fields[0]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD2, &s_settings.fields[1]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD3, &s_settings.fields[2]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD4, &s_settings.fields[3]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_HOUR_MODE, &s_settings.hour_mode);
  changed |= read_int_tuple(iter, MESSAGE_KEY_SHOW_AMPM, &s_settings.show_ampm);
  changed |= read_int_tuple(iter, MESSAGE_KEY_DIST_UNIT, &s_settings.dist_unit);
  changed |= read_int_tuple(iter, MESSAGE_KEY_SECONDARY_INFO, &s_settings.secondary_info);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD5, &s_settings.fields[4]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD6, &s_settings.fields[5]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD7, &s_settings.fields[6]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_FIELD8, &s_settings.fields[7]);
  changed |= read_int_tuple(iter, MESSAGE_KEY_TAP_VIBE, &s_settings.tap_vibe);
  changed |= read_int_tuple(iter, MESSAGE_KEY_VIBE_IN_QUIET, &s_settings.vibe_in_quiet);

  int hr_on_shake = s_hr_on_shake ? 1 : 0;
  if (read_int_tuple(iter, MESSAGE_KEY_HR_SHAKE, &hr_on_shake)) {
    s_hr_on_shake = (hr_on_shake != 0);
    persist_write_bool(HR_SHAKE_KEY, s_hr_on_shake);
    if (!s_hr_on_shake) {
      cancel_hr_measurement();
      refresh_heart_rate();
    }
  }

  if (!s_settings.secondary_info && s_second_fields) {
    s_second_fields = false;
    if (s_secondary_timer) {
      app_timer_cancel(s_secondary_timer);
      s_secondary_timer = NULL;
    }
  }

  if (s_settings.bg_option == BG_OPTION_FIX) {
    s_color_state.active_color = s_settings.bg_color;
  }

  if (changed) {
    persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  }

  Tuple *saved = dict_find(iter, MESSAGE_KEY_SAVED);
  if (saved && saved->value->int32) {
    light_enable_interaction();
  }

  update_tick_subscription();
  refresh_health();
  layer_mark_dirty(s_canvas);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_font_time  = fonts_load_custom_font(resource_get_handle(RES_FONT_TIME));
  s_font_date  = fonts_load_custom_font(resource_get_handle(RES_FONT_DATE));
  s_font_wday  = fonts_load_custom_font(resource_get_handle(RES_FONT_WDAY));
  s_font_info  = fonts_load_custom_font(resource_get_handle(RES_FONT_INFO));
  s_font_label = fonts_load_custom_font(resource_get_handle(RES_FONT_LABEL));
  s_font_ampm  = fonts_load_custom_font(resource_get_handle(RES_FONT_AMPM));

  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, canvas_update_proc);
  layer_add_child(root, s_canvas);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas);
  fonts_unload_custom_font(s_font_time);
  fonts_unload_custom_font(s_font_date);
  fonts_unload_custom_font(s_font_wday);
  fonts_unload_custom_font(s_font_info);
  fonts_unload_custom_font(s_font_label);
  fonts_unload_custom_font(s_font_ampm);
}

static void init(void) {
  time_t now = time(NULL);
  srand((unsigned int)now);
  struct tm *t = localtime(&now);
  if (t) s_now = *t;

  Settings loaded;
  if (persist_exists(SETTINGS_KEY) &&
      persist_read_data(SETTINGS_KEY, &loaded, sizeof(loaded)) == (int)sizeof(loaded) &&
      loaded.version == SETTINGS_VERSION) {
    s_settings = loaded;
  }

  ColorState cs;
  if (persist_exists(COLOR_STATE_KEY) &&
      persist_read_data(COLOR_STATE_KEY, &cs, sizeof(cs)) == (int)sizeof(cs)) {
    s_color_state = cs;
  } else {
    s_color_state.active_color = s_settings.bg_color;
  }

  if (persist_exists(HR_SHAKE_KEY)) {
    s_hr_on_shake = persist_read_bool(HR_SHAKE_KEY);
  }

  s_battery = battery_state_service_peek().charge_percent;
  refresh_health();

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(s_window, true);

  s_seconds_subscribed = is_seconds_needed();
  tick_timer_service_subscribe(s_seconds_subscribed ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  accel_tap_service_subscribe(tap_handler);
#if defined(PBL_HEALTH)
  s_health_subscribed = health_service_events_subscribe(health_handler, NULL);
#endif

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 64);
}

static void deinit(void) {
  cancel_hr_measurement();
  if (s_secondary_timer) {
    app_timer_cancel(s_secondary_timer);
    s_secondary_timer = NULL;
  }
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  accel_tap_service_unsubscribe();
#if defined(PBL_HEALTH)
  health_service_events_unsubscribe();
#endif
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
