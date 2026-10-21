const test = require('node:test');
const assert = require('node:assert/strict');
const { getCountryInfo, isPrivateIp, displayGameIp, UNKNOWN_INFO } = require('../lib/geo');
const { translator } = require('../lib/i18n');

test('geo: correctly detects private and local IPs', () => {
  assert.equal(isPrivateIp('127.0.0.1'), true);
  assert.equal(isPrivateIp('10.0.1.5'), true);
  assert.equal(isPrivateIp('172.23.0.1'), true);
  assert.equal(isPrivateIp('192.168.1.100'), true);
  assert.equal(isPrivateIp(null), true);
  assert.equal(isPrivateIp(''), true);
  assert.equal(isPrivateIp('88.156.211.71'), false);
  assert.equal(isPrivateIp('46.232.154.133'), false);
});

test('geo: shows the website IP when the game only recorded a private address', () => {
  assert.equal(displayGameIp('172.22.0.5', '46.53.226.69'), '46.53.226.69');
  assert.equal(displayGameIp('::ffff:10.0.0.8', '92.253.236.55'), '92.253.236.55');
  assert.equal(displayGameIp('46.53.226.69', '1.2.3.4'), '46.53.226.69');
  assert.equal(displayGameIp('172.22.0.5', '172.22.0.9'), '172.22.0.5');
  assert.equal(displayGameIp('', '46.53.226.69'), '');
  assert.equal(displayGameIp(null, null), '');
});

test('geo: resolves public IPs only when enabled, and hides countries only from the configured list', () => {
  const prevEnabled = process.env.GEOIP_ENABLED;
  const prevHidden = process.env.GEOIP_HIDDEN_COUNTRIES;
  const restore = (name, value) => {
    if (value === undefined) delete process.env[name];
    else process.env[name] = value;
  };
  try {
    process.env.GEOIP_ENABLED = '0';
    delete process.env.GEOIP_HIDDEN_COUNTRIES;
    assert.equal(getCountryInfo('88.156.211.71').isUnknown, true);

    process.env.GEOIP_ENABLED = '1';
    const pl = getCountryInfo('88.156.211.71');
    assert.equal(pl.countryCode, 'PL');
    assert.equal(pl.flagUrl, '/assets/images/flags/4x3/pl.svg');
    assert.equal(pl.isUnknown, false);

    const ua = getCountryInfo('92.253.236.55');
    assert.equal(ua.countryCode, 'UA');
    assert.equal(ua.flagUrl, '/assets/images/flags/4x3/ua.svg');
    assert.equal(ua.isUnknown, false);

    const br = getCountryInfo('45.227.249.240');
    assert.equal(br.countryCode, 'BR');
    assert.equal(br.flagUrl, '/assets/images/flags/4x3/br.svg');
    assert.equal(br.isUnknown, false);

    const ru = getCountryInfo('95.173.136.1');
    assert.equal(ru.countryCode, 'RU');
    assert.equal(ru.isUnknown, false);

    process.env.GEOIP_HIDDEN_COUNTRIES = 'RU';
    const hidden = getCountryInfo('109.252.0.1');
    assert.equal(hidden.countryCode, 'UNKNOWN');
    assert.equal(hidden.isUnknown, true);
    assert.equal(getCountryInfo('88.156.211.71').countryCode, 'PL');
  } finally {
    restore('GEOIP_ENABLED', prevEnabled);
    restore('GEOIP_HIDDEN_COUNTRIES', prevHidden);
  }
});


test('geo & i18n: translations for rankings and countries work across all supported languages', () => {
  const tUk = translator('uk');
  const tEn = translator('en');
  const tEs = translator('es');
  const tPt = translator('pt');

  assert.equal(tUk('Unknown'), 'Невідомо');
  assert.equal(tEn('Unknown'), 'Unknown');
  assert.equal(tEs('Unknown'), 'Desconocido');
  assert.equal(tPt('Unknown'), 'Desconhecido');

  assert.equal(tUk('Pais'), 'Країна');
  assert.equal(tEn('Pais'), 'Country');
  assert.equal(tEs('Pais'), 'País');
  assert.equal(tPt('Pais'), 'País');

  assert.equal(tUk('Ukraine'), 'Україна');
  assert.equal(tUk('Poland'), 'Польща');
  assert.equal(tUk('En linea'), 'В мережі');
  assert.equal(tUk('Desconectado'), 'Офлайн');
  assert.equal(translator('de')('Pais'), 'País');
});
