'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '..');
const pkg = JSON.parse(fs.readFileSync(path.join(root, 'pebble/package.json'), 'utf8'));
const config = JSON.parse(fs.readFileSync(path.join(root, 'pebble/src/pkjs/config.json'), 'utf8'));

// 1. Validate Pebble manifest
assert.equal(pkg.pebble.displayName, 'Blokk');
assert.deepEqual(pkg.pebble.targetPlatforms, ['basalt', 'chalk', 'emery', 'gabbro']);
assert.equal(pkg.pebble.watchapp.watchface, true);
assert.ok(pkg.pebble.uuid && pkg.pebble.uuid !== '57a099b6-508f-41fb-badc-1371da1492d9', 'Must have unique UUID');

// 2. Validate every messageKey in config.json exists in package.json
const keySet = new Set(pkg.pebble.messageKeys);
const configKeys = [];
for (const entry of config) {
  if (entry.messageKey) configKeys.push(entry.messageKey);
  if (Array.isArray(entry.items)) {
    for (const item of entry.items) {
      if (item.messageKey) configKeys.push(item.messageKey);
    }
  }
}
assert.ok(configKeys.length >= 17, 'Expected at least 17 configurable keys');
for (const k of configKeys) {
  assert.ok(keySet.has(k), `Config key ${k} missing from package.json messageKeys`);
}

// 3. Test companion.js in a mocked Pebble environment
const sentMessages = [];
const listeners = {};
const store = {};

global.localStorage = {
  getItem: (k) => (Object.prototype.hasOwnProperty.call(store, k) ? store[k] : null),
  setItem: (k, v) => { store[k] = String(v); }
};
global.Pebble = {
  addEventListener: (ev, fn) => { listeners[ev] = fn; },
  sendAppMessage: (data, ok) => { sentMessages.push(data); if (ok) ok(); },
  openURL: () => {}
};

// Mock require('message_keys') and require('@rebble/clay')
const Module = require('node:module');
const origLoad = Module._load;
const mockKeys = {};
pkg.pebble.messageKeys.forEach((k, idx) => { mockKeys[k] = 10000 + idx; });

Module._load = function (request, parent, isMain) {
  if (request === 'message_keys') return mockKeys;
  if (request === '@rebble/clay') {
    return function MockClay() {
      this.generateUrl = () => 'data:text/html,mock';
      this.getSettings = (resp) => JSON.parse(resp);
    };
  }
  return origLoad.apply(this, arguments);
};

try {
  const comp = require(path.join(root, 'pebble/src/pkjs/companion.js'));

  // Trigger ready -> sends default payload without SAVED flag
  listeners.ready();
  assert.equal(sentMessages.length, 1);
  assert.equal(sentMessages[0][mockKeys.BG_OPTION], 5);
  assert.equal(sentMessages[0][mockKeys.FIELD1], 1);
  assert.equal(sentMessages[0][mockKeys.FIELD3], 10, 'VIB is the default top-right field');
  assert.equal(sentMessages[0][mockKeys.SAVED], undefined);

  // Trigger webviewclosed -> saves and sends payload with SAVED=1
  const mockResponse = JSON.stringify({
    BG_OPTION: { value: '0' },
    BG_COLOR: { value: '4' },
    BG_GRADIENT: { value: true },
    FIELD1: { value: '1' },
    FIELD2: { value: '2' },
    FIELD3: { value: '4' },
    FIELD4: { value: '3' },
    HOUR_MODE: { value: '2' },
    SHOW_AMPM: { value: true },
    DIST_UNIT: { value: '2' },
    SECONDARY_INFO: { value: true },
    FIELD5: { value: '6' },
    FIELD6: { value: '9' },
    FIELD7: { value: '5' },
    FIELD8: { value: '7' },
    TAP_VIBE: { value: true },
    VIBE_IN_QUIET: { value: false },
    HR_SHAKE: { value: true }
  });
  listeners.webviewclosed({ response: mockResponse });
  assert.equal(sentMessages.length, 2);
  assert.equal(sentMessages[1][mockKeys.BG_OPTION], 0);
  assert.equal(sentMessages[1][mockKeys.BG_COLOR], 4);
  assert.equal(sentMessages[1][mockKeys.BG_GRADIENT], 1);
  assert.equal(sentMessages[1][mockKeys.HOUR_MODE], 2);
  assert.equal(sentMessages[1][mockKeys.SHOW_AMPM], 1);
  assert.equal(sentMessages[1][mockKeys.DIST_UNIT], 2);
  assert.equal(sentMessages[1][mockKeys.SECONDARY_INFO], 1);
  assert.equal(sentMessages[1][mockKeys.HR_SHAKE], 1);
  assert.equal(sentMessages[1][mockKeys.SAVED], 1);

  // Verify persistence in localStorage
  const persisted = JSON.parse(store['blokk.settings']);
  assert.equal(persisted.BG_COLOR, 4);
  assert.equal(persisted.BG_GRADIENT, 1);
  assert.equal(comp.toBoolInt('true'), 1);
  assert.equal(comp.toBoolInt(false), 0);
  // Emery (HR sensor) keeps BPM / SEC as the 2nd / 6th fields
  assert.equal(sentMessages[0][mockKeys.FIELD2], 2, 'BPM is the default 2nd field with an HR sensor');
  assert.equal(sentMessages[0][mockKeys.FIELD6], 9, 'SEC is the default 6th field');

  // Watches without an HR sensor: fresh install -> ACT as 2nd field, SEC stays in the wrist-flick set
  for (const platform of ['basalt', 'chalk', 'gabbro']) {
    for (const k of Object.keys(store)) delete store[k];
    sentMessages.length = 0;
    global.Pebble.getActiveWatchInfo = () => ({ platform });
    delete require.cache[require.resolve(path.join(root, 'pebble/src/pkjs/companion.js'))];
    require(path.join(root, 'pebble/src/pkjs/companion.js'));
    listeners.ready();
    assert.equal(sentMessages[0][mockKeys.FIELD2], 7, `${platform}: ACT is the default 2nd field`);
    assert.equal(sentMessages[0][mockKeys.FIELD6], 9, `${platform}: SEC is the default 6th field`);
    assert.equal(sentMessages[0][mockKeys.FIELD3], 10, `${platform}: VIB is the default 3rd field`);
  }
  delete global.Pebble.getActiveWatchInfo;
} finally {
  Module._load = origLoad;
}

console.log('All Blokk companion & manifest tests passed.');
