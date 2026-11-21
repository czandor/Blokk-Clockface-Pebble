const fs = require('node:fs');
const path = require('node:path');

const root = path.resolve(__dirname, '../pebble');
const version = '1.0.0';

const keys = [
  'BG_OPTION',
  'BG_COLOR',
  'BG_GRADIENT',
  'FIELD1',
  'FIELD2',
  'FIELD3',
  'FIELD4',
  'HOUR_MODE',
  'SHOW_AMPM',
  'DIST_UNIT',
  'SECONDARY_INFO',
  'FIELD5',
  'FIELD6',
  'FIELD7',
  'FIELD8',
  'TAP_VIBE',
  'VIBE_IN_QUIET',
  'SAVED',
  'HR_SHAKE'
];

const media = [
  // Emery (Pebble Time 2, 200x228)
  { type: 'font', name: 'FONT_TIME_84', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[0-9]', targetPlatforms: ['emery'] },
  { type: 'font', name: 'FONT_DATE_22', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[A-Z0-9 ]', targetPlatforms: ['emery'] },
  { type: 'font', name: 'FONT_WDAY_14', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[A-Z]', targetPlatforms: ['emery'] },
  { type: 'font', name: 'FONT_INFO_22', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[0-9.\\-]', targetPlatforms: ['emery'] },
  { type: 'font', name: 'FONT_LABEL_15', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[A-Z]', targetPlatforms: ['emery'] },
  { type: 'font', name: 'FONT_AMPM_24', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[AMP]', targetPlatforms: ['emery'] },
  // Basalt (Pebble Time / Time Steel, 144x168)
  { type: 'font', name: 'FONT_TIME_60', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[0-9]', targetPlatforms: ['basalt'] },
  { type: 'font', name: 'FONT_DATE_16', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[A-Z0-9 ]', targetPlatforms: ['basalt'] },
  { type: 'font', name: 'FONT_WDAY_11', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[A-Z]', targetPlatforms: ['basalt'] },
  { type: 'font', name: 'FONT_INFO_16', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[0-9.\\-]', targetPlatforms: ['basalt'] },
  { type: 'font', name: 'FONT_LABEL_12', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[A-Z]', targetPlatforms: ['basalt'] },
  { type: 'font', name: 'FONT_AMPM_18', file: 'fonts/AmericanCaptain.ttf', characterRegex: '[AMP]', targetPlatforms: ['basalt'] }
];

fs.writeFileSync(
  path.join(root, 'package.json'),
  JSON.stringify(
    {
      name: 'blokk',
      author: 'Andor Czafik',
      version,
      private: true,
      description: 'Blokk watchface for Pebble Time and Pebble Time 2, ported from the original Fitbit clockface (Design: Zoltan Nagy, Code: Andor Czafik).',
      dependencies: { 'pebble-clay': '1.0.4' },
      pebble: {
        displayName: 'Blokk',
        uuid: '3fb5fda5-0820-4c60-8adf-c07541a3870d',
        sdkVersion: '3',
        enableMultiJS: true,
        targetPlatforms: ['basalt', 'emery'],
        capabilities: ['configurable', 'health'],
        watchapp: { watchface: true },
        messageKeys: keys,
        resources: { media }
      }
    },
    null,
    2
  ) + '\n'
);

const bgOptions = [
  { label: 'By the time of day', value: '5' },
  { label: 'Choose one color', value: '0' },
  { label: 'Random every day', value: '4' },
  { label: 'Random every hour', value: '2' },
  { label: 'Random every minute', value: '1' },
  { label: 'Random on wrist flick', value: '3' }
];

const bgColors = [
  { label: 'Red', value: '0' },
  { label: 'Orange', value: '1' },
  { label: 'Yellow', value: '2' },
  { label: 'Green', value: '3' },
  { label: 'Turquoise', value: '4' },
  { label: 'Blue', value: '5' },
  { label: 'Dark Blue', value: '6' },
  { label: 'Pink', value: '7' },
  { label: 'Purple', value: '8' }
];

const fieldOptions = [
  { label: 'Nothing', value: '0' },
  { label: 'STP: Today\'s steps', value: '1' },
  { label: 'BPM: Heart rate', value: '2' },
  { label: 'BTT: Battery %', value: '3' },
  { label: 'BTT: Battery, if low (<=30%)', value: '8' },
  { label: 'DST: Distance (km or mi)', value: '4' },
  { label: 'SLP: Sleep hours', value: '5' },
  { label: 'CAL: Calories burned', value: '6' },
  { label: 'ACT: Active minutes', value: '7' },
  { label: 'SEC: Seconds', value: '9' }
];

const config = [
  { type: 'heading', defaultValue: 'Blokk' },
  {
    type: 'text',
    defaultValue: `Pebble Time / Time 2 · ${version}<br>Design: Zoltan Nagy (Zo Nagy) · Code: Andor Czafik`
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Background Color' },
      {
        type: 'select',
        messageKey: 'BG_OPTION',
        label: 'Color selection method',
        defaultValue: '5',
        options: bgOptions
      },
      {
        type: 'select',
        messageKey: 'BG_COLOR',
        label: 'Fixed background color',
        defaultValue: '2',
        options: bgColors
      },
      {
        type: 'toggle',
        messageKey: 'BG_GRADIENT',
        label: 'Gradient background',
        defaultValue: false
      },
      {
        type: 'text',
        defaultValue: 'Time of day mode shifts from red/orange at dawn, yellow in the morning, turquoise/blue in the afternoon, to purple at night.'
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Info Fields' },
      { type: 'select', messageKey: 'FIELD1', label: 'Top-left field', defaultValue: '1', options: fieldOptions },
      { type: 'select', messageKey: 'FIELD3', label: 'Top-right field', defaultValue: '7', options: fieldOptions },
      { type: 'select', messageKey: 'FIELD2', label: 'Middle-bottom field', defaultValue: '2', options: fieldOptions },
      { type: 'select', messageKey: 'FIELD4', label: 'Bottom field', defaultValue: '3', options: fieldOptions }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Extra Info Fields (Wrist Flick)' },
      {
        type: 'toggle',
        messageKey: 'SECONDARY_INFO',
        label: 'Enable secondary info fields',
        defaultValue: false
      },
      { type: 'select', messageKey: 'FIELD5', label: 'Flick: Top-left field', defaultValue: '6', options: fieldOptions },
      { type: 'select', messageKey: 'FIELD7', label: 'Flick: Top-right field', defaultValue: '5', options: fieldOptions },
      { type: 'select', messageKey: 'FIELD6', label: 'Flick: Middle-bottom field', defaultValue: '9', options: fieldOptions },
      { type: 'select', messageKey: 'FIELD8', label: 'Flick: Bottom field', defaultValue: '4', options: fieldOptions },
      {
        type: 'toggle',
        messageKey: 'HR_SHAKE',
        label: 'Measure heart rate on shake',
        defaultValue: false
      },
      {
        type: 'text',
        defaultValue: 'A wrist flick requests up to 45 seconds of faster heart rate sampling (1 Hz) until two consistent readings confirm BPM. During measurement the BPM label blinks. Frequent use drains the battery faster.'
      },
      {
        type: 'toggle',
        messageKey: 'TAP_VIBE',
        label: 'Vibrate on wrist flick',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'VIBE_IN_QUIET',
        label: 'Vibrate on flick during Quiet Time',
        defaultValue: false
      },
      {
        type: 'text',
        defaultValue: 'Flick your wrist to switch to the secondary info fields for 20 seconds (flick again to return immediately).'
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Time & Units' },
      {
        type: 'select',
        messageKey: 'HOUR_MODE',
        label: 'Time format',
        defaultValue: '0',
        options: [
          { label: 'Watch preference', value: '0' },
          { label: '24h', value: '1' },
          { label: '12h', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'SHOW_AMPM',
        label: 'Show AM/PM in 12h mode',
        defaultValue: false
      },
      {
        type: 'select',
        messageKey: 'DIST_UNIT',
        label: 'Distance unit',
        defaultValue: '0',
        options: [
          { label: 'Metric (km)', value: '1' },
          { label: 'US (miles)', value: '2' },
          { label: 'Default (km)', value: '0' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Support' },
      {
        type: 'text',
        defaultValue: 'This watchface is completely free. If you like it, you can support our work here: <a href="https://czandor.hu/paypal/blokk">Buy us a coffee</a>'
      }
    ]
  },
  { type: 'submit', defaultValue: 'Save settings' }
];

fs.writeFileSync(path.join(root, 'src/pkjs/config.json'), JSON.stringify(config, null, 2) + '\n');
console.log('Prepared Blokk Emery manifest and offline Clay settings.');
