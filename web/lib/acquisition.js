// First-touch acquisition. The cookie remembers the landing; the account row
// is written once, at registration, and never replaced.
const SOURCE_COOKIE = 'mu_src';
const SOURCE_MAX_AGE_MS = 90 * 24 * 60 * 60 * 1000;
const SOURCE_CHANNELS = Object.freeze(['direct', 'organic', 'social', 'referral', 'campaign']);

const HOST_RULES = [
  { channel: 'organic', source: 'google', hosts: ['google.com', 'google.co.uk', 'google.com.ua', 'google.com.br'] },
  { channel: 'organic', source: 'bing', hosts: ['bing.com'] },
  { channel: 'organic', source: 'duckduckgo', hosts: ['duckduckgo.com'] },
  { channel: 'organic', source: 'yahoo', hosts: ['yahoo.com', 'search.yahoo.com'] },
  { channel: 'organic', source: 'yandex', hosts: ['yandex.com', 'yandex.ru', 'yandex.ua', 'ya.ru'] },
  { channel: 'social', source: 'discord', hosts: ['discord.com', 'discord.gg'] },
  { channel: 'social', source: 'youtube', hosts: ['youtube.com', 'youtu.be'] },
  { channel: 'social', source: 'facebook', hosts: ['facebook.com', 'fb.com', 'fb.me'] },
  { channel: 'social', source: 'instagram', hosts: ['instagram.com'] },
  { channel: 'social', source: 'tiktok', hosts: ['tiktok.com'] },
  { channel: 'social', source: 'reddit', hosts: ['reddit.com'] },
  { channel: 'social', source: 'x', hosts: ['x.com', 'twitter.com', 't.co'] },
  { channel: 'social', source: 'telegram', hosts: ['t.me', 'telegram.org', 'telegram.me'] },
  { channel: 'social', source: 'twitch', hosts: ['twitch.tv'] },
  { channel: 'social', source: 'vk', hosts: ['vk.com'] },
  { channel: 'social', source: 'steam', hosts: ['steamcommunity.com', 'steampowered.com'] }
];

function clip(value, max) {
  return String(value ?? '').replace(/[\u0000-\u001f\u007f]/g, '').trim().slice(0, max);
}

function normalizeHost(value) {
  const host = clip(value, 255).toLowerCase().replace(/:\d+$/, '').replace(/\.$/, '');
  return host.startsWith('www.') ? host.slice(4) : host;
}

function hostMatches(host, names) {
  if (host === 'google.com' || /(^|\.)google\.[a-z]{2,3}(\.[a-z]{2})?$/.test(host)) {
    return names.includes('google.com');
  }
  return names.some((name) => host === name || host.endsWith('.' + name));
}

function hostOf(url) {
  try {
    return normalizeHost(new URL(String(url || '')).hostname);
  } catch {
    return '';
  }
}

function ownHostList(headerHost, forwardedHost) {
  const found = [];
  for (const value of [forwardedHost, headerHost]) {
    const host = normalizeHost(String(value || '').split(',')[0]);
    if (host && !found.includes(host)) found.push(host);
  }
  return found;
}

function isOwnHost(host, ownHosts) {
  const name = normalizeHost(host);
  if (!name) return false;
  return ownHosts.some((own) => name === own || name.endsWith('.' + own));
}

function cleanPath(value) {
  const text = clip(value, 300).split('?')[0];
  if (!text.startsWith('/') || text.includes('..') || text.includes('\\')) return '/';
  return text.slice(0, 255) || '/';
}

function token(value, max) {
  return clip(value, max).toLowerCase();
}

function firstQuery(query, key) {
  const value = query && query[key];
  return Array.isArray(value) ? value[0] : value;
}

function classifyVisit({ referer, query, landingPath, ownHosts }) {
  const source = token(firstQuery(query, 'utm_source'), 64);
  const medium = token(firstQuery(query, 'utm_medium'), 32);
  const campaign = token(firstQuery(query, 'utm_campaign'), 64);
  const landing = cleanPath(landingPath);
  const refHost = hostOf(referer);
  const own = isOwnHost(refHost, ownHosts || []);

  if (source) {
    return {
      channel: 'campaign',
      source,
      medium,
      campaign,
      landing_path: landing,
      referrer_host: own ? '' : refHost
    };
  }

  // A click inside the site is not an acquisition. Wait for the original landing.
  if (own) return null;
  if (!refHost) {
    return { channel: 'direct', source: 'direct', medium: '', campaign: '', landing_path: landing, referrer_host: '' };
  }

  for (const rule of HOST_RULES) {
    if (hostMatches(refHost, rule.hosts)) {
      return {
        channel: rule.channel,
        source: rule.source,
        medium: rule.channel,
        campaign: '',
        landing_path: landing,
        referrer_host: refHost
      };
    }
  }

  return {
    channel: 'referral',
    source: refHost.slice(0, 64),
    medium: 'referral',
    campaign: '',
    landing_path: landing,
    referrer_host: refHost
  };
}

function encodeSource(attr, now = Date.now()) {
  return Buffer.from(JSON.stringify({
    v: 1,
    channel: attr.channel,
    source: attr.source,
    medium: attr.medium || '',
    campaign: attr.campaign || '',
    landing_path: attr.landing_path || '/',
    referrer_host: attr.referrer_host || '',
    ts: Number.isFinite(attr.ts) ? attr.ts : now
  }), 'utf8').toString('base64url');
}

function decodeSource(raw) {
  if (typeof raw !== 'string' || !raw || raw.length > 700) return null;
  let data;
  try {
    data = JSON.parse(Buffer.from(raw, 'base64url').toString('utf8'));
  } catch {
    return null;
  }
  if (!data || typeof data !== 'object' || (data.v != null && data.v !== 1)) return null;
  if (!SOURCE_CHANNELS.includes(data.channel)) return null;
  const source = clip(data.source, 64);
  if (!source) return null;
  const ts = Number(data.ts);
  return {
    channel: data.channel,
    source,
    medium: clip(data.medium, 32),
    campaign: clip(data.campaign, 64),
    landing_path: cleanPath(data.landing_path),
    referrer_host: normalizeHost(data.referrer_host).slice(0, 255),
    ts: Number.isFinite(ts) ? ts : null
  };
}

function sourceCapturedAt(ts, now = Date.now()) {
  const value = Number(ts);
  if (!Number.isFinite(value) || value < now - SOURCE_MAX_AGE_MS || value > now + 5 * 60 * 1000) return new Date(now);
  return new Date(value);
}

function sourceLabel(attr) {
  if (!attr || !attr.source) return '';
  if (attr.channel === 'direct') return 'direct';
  if (attr.channel === 'organic') return `${attr.source} · organic`;
  if (attr.channel === 'campaign') return attr.campaign ? `${attr.source} · ${attr.campaign}` : attr.source;
  return attr.source;
}

function sourceTitle(attr, when) {
  if (!attr || !attr.channel) return '';
  const parts = [attr.channel];
  if (attr.medium && attr.medium !== attr.channel) parts.push(attr.medium);
  if (attr.campaign) parts.push(attr.campaign);
  if (attr.landing_path) parts.push(attr.landing_path);
  if (attr.referrer_host) parts.push(attr.referrer_host);
  if (when) parts.push(when);
  return parts.join(' · ');
}

function readRequestCookie(header, name) {
  if (!header || !name) return '';
  for (const part of String(header).split(';')) {
    const eq = part.indexOf('=');
    if (eq === -1) continue;
    if (part.slice(0, eq).trim() !== name) continue;
    try {
      return decodeURIComponent(part.slice(eq + 1).trim());
    } catch {
      return '';
    }
  }
  return '';
}

function shouldCaptureSource(req) {
  if (!req || req.method !== 'GET') return false;
  const dest = String(req.headers && req.headers['sec-fetch-dest'] || '');
  if (dest && dest !== 'document') return false;
  const path = String(req.path || '/');
  if (path.startsWith('/admin') || path.startsWith('/client') || path.startsWith('/locales')) return false;
  if (/\.[a-z0-9]{1,8}$/i.test(path)) return false;
  return true;
}

module.exports = {
  SOURCE_COOKIE,
  SOURCE_MAX_AGE_MS,
  SOURCE_CHANNELS,
  classifyVisit,
  encodeSource,
  decodeSource,
  sourceCapturedAt,
  sourceLabel,
  sourceTitle,
  ownHostList,
  readRequestCookie,
  shouldCaptureSource,
  cleanPath
};
