const test = require('node:test');
const assert = require('node:assert/strict');
const {
  classifyVisit,
  encodeSource,
  decodeSource,
  sourceLabel,
  sourceTitle,
  sourceCapturedAt,
  ownHostList,
  readRequestCookie,
  shouldCaptureSource,
  SOURCE_MAX_AGE_MS
} = require('../lib/acquisition');

const OWN = ['muonline.digitalsplace.com'];

test('acquisition: classifies search, social, referral and direct', () => {
  const google = classifyVisit({ referer: 'https://www.google.com/search?q=mu', query: {}, landingPath: '/', ownHosts: OWN });
  assert.equal(google.channel, 'organic');
  assert.equal(google.source, 'google');
  assert.equal(google.referrer_host, 'google.com');

  const discord = classifyVisit({ referer: 'https://discord.gg/abc', query: {}, landingPath: '/news', ownHosts: OWN });
  assert.equal(discord.channel, 'social');
  assert.equal(discord.source, 'discord');
  assert.equal(discord.landing_path, '/news');

  const forum = classifyVisit({ referer: 'https://forum.example.com/topic/1', query: {}, landingPath: '/', ownHosts: OWN });
  assert.equal(forum.channel, 'referral');
  assert.equal(forum.source, 'forum.example.com');

  const direct = classifyVisit({ referer: '', query: {}, landingPath: '/register', ownHosts: OWN });
  assert.equal(direct.channel, 'direct');
  assert.equal(direct.source, 'direct');
  assert.equal(direct.landing_path, '/register');
});

test('acquisition: keeps the explicit campaign and ignores later internal clicks', () => {
  const campaign = classifyVisit({
    referer: 'https://discord.com/channels/1',
    query: { utm_source: 'Discord', utm_medium: 'social', utm_campaign: 'Opening' },
    landingPath: '/?utm_source=Discord',
    ownHosts: OWN
  });
  assert.equal(campaign.channel, 'campaign');
  assert.equal(campaign.source, 'discord');
  assert.equal(campaign.medium, 'social');
  assert.equal(campaign.campaign, 'opening');
  assert.equal(campaign.landing_path, '/');

  assert.equal(classifyVisit({
    referer: 'https://muonline.digitalsplace.com/news',
    query: {},
    landingPath: '/register',
    ownHosts: OWN
  }), null);
});

test('acquisition: does not treat a lookalike host as the site or as google', () => {
  const lookalike = classifyVisit({
    referer: 'https://notmuonline.digitalsplace.com/',
    query: {},
    landingPath: '/',
    ownHosts: OWN
  });
  assert.equal(lookalike.channel, 'referral');

  const fakeGoogle = classifyVisit({
    referer: 'https://notgoogle.com/',
    query: {},
    landingPath: '/',
    ownHosts: OWN
  });
  assert.equal(fakeGoogle.channel, 'referral');
  assert.equal(fakeGoogle.source, 'notgoogle.com');
});

test('acquisition: cookie roundtrip keeps the first landing', () => {
  const attr = classifyVisit({ referer: 'https://youtube.com/watch?v=1', query: {}, landingPath: '/', ownHosts: OWN });
  attr.ts = 1_700_000_000_000;
  const decoded = decodeSource(encodeSource(attr));
  assert.deepEqual(decoded, { ...attr, ts: attr.ts });
  assert.equal(decodeSource('not-json'), null);
  assert.equal(decodeSource(''), null);
  assert.equal(sourceLabel(decoded), 'youtube');
  assert.equal(sourceLabel({ channel: 'organic', source: 'google' }), 'google · organic');
  assert.equal(sourceLabel({ channel: 'campaign', source: 'discord', campaign: 'opening' }), 'discord · opening');
  assert.match(sourceTitle(decoded, '18 Oct 2026'), /social · \/ · youtube.com · 18 Oct 2026/);
});

test('acquisition: rejects a visit timestamp outside the cookie lifetime', () => {
  const now = 1_800_000_000_000;
  assert.equal(sourceCapturedAt(now - 1000, now).getTime(), now - 1000);
  assert.equal(sourceCapturedAt(now - SOURCE_MAX_AGE_MS - 1, now).getTime(), now);
  assert.equal(sourceCapturedAt('nope', now).getTime(), now);
});

test('acquisition: reads the source cookie and skips assets and admin', () => {
  assert.equal(readRequestCookie('lang=en; mu_src=abc%3D; other=1', 'mu_src'), 'abc=');
  assert.deepEqual(ownHostList('muonline.digitalsplace.com:8080', 'muonline.digitalsplace.com, edge'), ['muonline.digitalsplace.com']);
  assert.equal(shouldCaptureSource({ method: 'GET', path: '/', headers: {} }), true);
  assert.equal(shouldCaptureSource({ method: 'GET', path: '/admin/accounts', headers: {} }), false);
  assert.equal(shouldCaptureSource({ method: 'GET', path: '/css/app.css', headers: {} }), false);
  assert.equal(shouldCaptureSource({ method: 'POST', path: '/register', headers: {} }), false);
  assert.equal(shouldCaptureSource({ method: 'GET', path: '/', headers: { 'sec-fetch-dest': 'script' } }), false);
});
