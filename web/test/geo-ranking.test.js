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

test('geo: shows the website IP when the game only saw the gateway', () => {
  assert.equal(displayGameIp('172.22.0.5', '46.53.226.69'), '46.53.226.69');
  assert.equal(displayGameIp('::ffff:10.0.0.8', '92.253.236.55'), '92.253.236.55');
  assert.equal(displayGameIp('46.53.226.69', '1.2.3.4'), '46.53.226.69');
  assert.equal(displayGameIp('172.22.0.5', '172.22.0.9'), '172.22.0.5');
  assert.equal(displayGameIp('', '46.53.226.69'), '');
  assert.equal(displayGameIp(null, null), '');
});

test('geo: resolves real public IPs to proper country flags', () => {
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
});

test('geo: strictly masks Russian IPs as unknown (росія - unknown)', () => {
  // Moscow public IP
  const ru1 = getCountryInfo('95.173.136.1');
  assert.equal(ru1.countryCode, 'UNKNOWN');
  assert.equal(ru1.countryName, 'Unknown');
  assert.equal(ru1.flagUrl, '/assets/images/flags/4x3/unknown.svg');
  assert.equal(ru1.isUnknown, true);

  // Rostelecom Russian IP
  const ru2 = getCountryInfo('109.252.0.1');
  assert.equal(ru2.countryCode, 'UNKNOWN');
  assert.equal(ru2.countryName, 'Unknown');
  assert.equal(ru2.flagUrl, '/assets/images/flags/4x3/unknown.svg');
  assert.equal(ru2.isUnknown, true);
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
});
