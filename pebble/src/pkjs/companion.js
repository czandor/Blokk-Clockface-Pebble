'use strict';

var Clay = require('@rebble/clay');
var keys = require('message_keys');

function clayCustomFn() {
  this.on(this.EVENTS.AFTER_BUILD, function () {
    var bgOption = this.getItemByMessageKey('BG_OPTION');
    var bgColor = this.getItemByMessageKey('BG_COLOR');
    var secToggle = this.getItemByMessageKey('SECONDARY_INFO');
    var secFields = ['FIELD5', 'FIELD6', 'FIELD7', 'FIELD8'].map(function (k) {
      return this.getItemByMessageKey(k);
    }, this);

    function syncBgColor() {
      if (!bgOption || !bgColor) return;
      if (String(bgOption.get()) === '0') bgColor.show();
      else bgColor.hide();
    }

    function syncSecondary() {
      if (!secToggle) return;
      var enabled = !!secToggle.get();
      secFields.forEach(function (item) {
        if (!item) return;
        if (enabled) item.show();
        else item.hide();
      });
    }

    if (bgOption) bgOption.on('change', syncBgColor);
    if (secToggle) secToggle.on('change', syncSecondary);
    syncBgColor();
    syncSecondary();
  });
}

var clay = new Clay(require('./config.json'), clayCustomFn, { autoHandleEvents: false });

var defaults = {
  BG_OPTION: 5,
  BG_COLOR: 2,
  BG_GRADIENT: 0,
  FIELD1: 1,
  FIELD2: 2,
  FIELD3: 10, // VIB (Quiet Time indicator)
  FIELD4: 3,
  HOUR_MODE: 0,
  SHOW_AMPM: 0,
  DIST_UNIT: 0,
  SECONDARY_INFO: 0,
  FIELD5: 6,
  FIELD6: 9,
  FIELD7: 5,
  FIELD8: 4,
  TAP_VIBE: 1,
  VIBE_IN_QUIET: 0,
  HR_SHAKE: 0
};

var settings = {};
var queue = [];
var sending = false;

function stored(key, fallback) {
  try {
    return JSON.parse(localStorage.getItem(key)) || fallback;
  } catch (e) {
    return fallback;
  }
}

// Pebble Time (basalt), Time Round (chalk) and Round 2 (gabbro) have no heart-rate sensor.
var NO_HR_PLATFORMS = ['basalt', 'chalk', 'gabbro'];

function watchLacksHrSensor() {
  try {
    if (typeof Pebble !== 'undefined' && typeof Pebble.getActiveWatchInfo === 'function') {
      var info = Pebble.getActiveWatchInfo();
      return !!info && NO_HR_PLATFORMS.indexOf(info.platform) !== -1;
    }
  } catch (e) {}
  return false;
}

function loadSettings() {
  var saved = stored('blokk.settings', null);
  var noHr = watchLacksHrSensor();
  var src = saved || {};
  Object.keys(defaults).forEach(function (k) {
    if (src[k] !== undefined) {
      settings[k] = src[k];
    } else if (noHr && k === 'FIELD2') {
      settings[k] = 9; // SEC instead of BPM (no HR sensor)
    } else if (noHr && k === 'FIELD6') {
      settings[k] = 0; // Nothing instead of duplicate SEC in secondary fields
    } else {
      settings[k] = defaults[k];
    }
  });
  if (!saved && noHr && typeof clay.setSettings === 'function') {
    try {
      clay.setSettings({ FIELD2: '9', FIELD6: '0' });
    } catch (e) {}
  }
}

loadSettings();

function send(message) {
  queue.push({ data: message, attempts: 0 });
  drain();
}

function drain() {
  if (sending || !queue.length) return;
  sending = true;
  var item = queue[0];
  Pebble.sendAppMessage(
    item.data,
    function () {
      queue.shift();
      sending = false;
      drain();
    },
    function () {
      sending = false;
      if (++item.attempts >= 3) queue.shift();
      setTimeout(drain, 1500);
    }
  );
}

function toInt(val, fallback) {
  var n = parseInt(val, 10);
  return isNaN(n) ? fallback : n;
}

function toBoolInt(val) {
  if (val === true || val === 1 || val === '1' || val === 'true') return 1;
  return 0;
}

function buildPayload(savedFlag) {
  var msg = {};
  msg[keys.BG_OPTION] = toInt(settings.BG_OPTION, defaults.BG_OPTION);
  msg[keys.BG_COLOR] = toInt(settings.BG_COLOR, defaults.BG_COLOR);
  msg[keys.BG_GRADIENT] = toBoolInt(settings.BG_GRADIENT);
  msg[keys.FIELD1] = toInt(settings.FIELD1, defaults.FIELD1);
  msg[keys.FIELD2] = toInt(settings.FIELD2, defaults.FIELD2);
  msg[keys.FIELD3] = toInt(settings.FIELD3, defaults.FIELD3);
  msg[keys.FIELD4] = toInt(settings.FIELD4, defaults.FIELD4);
  msg[keys.HOUR_MODE] = toInt(settings.HOUR_MODE, defaults.HOUR_MODE);
  msg[keys.SHOW_AMPM] = toBoolInt(settings.SHOW_AMPM);
  msg[keys.DIST_UNIT] = toInt(settings.DIST_UNIT, defaults.DIST_UNIT);
  msg[keys.SECONDARY_INFO] = toBoolInt(settings.SECONDARY_INFO);
  msg[keys.FIELD5] = toInt(settings.FIELD5, defaults.FIELD5);
  msg[keys.FIELD6] = toInt(settings.FIELD6, defaults.FIELD6);
  msg[keys.FIELD7] = toInt(settings.FIELD7, defaults.FIELD7);
  msg[keys.FIELD8] = toInt(settings.FIELD8, defaults.FIELD8);
  msg[keys.TAP_VIBE] = toBoolInt(settings.TAP_VIBE);
  msg[keys.VIBE_IN_QUIET] = toBoolInt(settings.VIBE_IN_QUIET);
  msg[keys.HR_SHAKE] = toBoolInt(settings.HR_SHAKE);
  if (savedFlag) msg[keys.SAVED] = 1;
  return msg;
}

Pebble.addEventListener('ready', function () {
  send(buildPayload(false));
});

Pebble.addEventListener('showConfiguration', function () {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (!e || !e.response) return;
  var raw = clay.getSettings(e.response, false);
  var parsed = {};
  Object.keys(defaults).forEach(function (k) {
    if (raw[k] !== undefined) {
      var val = raw[k].value !== undefined ? raw[k].value : raw[k];
      if (k === 'BG_GRADIENT' || k === 'SHOW_AMPM' || k === 'SECONDARY_INFO' || k === 'TAP_VIBE' || k === 'VIBE_IN_QUIET' || k === 'HR_SHAKE') {
        parsed[k] = toBoolInt(val);
      } else {
        parsed[k] = toInt(val, defaults[k]);
      }
    } else {
      parsed[k] = defaults[k];
    }
  });
  settings = parsed;
  try {
    localStorage.setItem('blokk.settings', JSON.stringify(settings));
  } catch (err) {
    console.log('Storage error: ' + err.message);
  }
  send(buildPayload(true));
});

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { defaults: defaults, buildPayload: buildPayload, toInt: toInt, toBoolInt: toBoolInt };
}
