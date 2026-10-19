const geoipLite = (() => {
  try {
    return require('geoip-lite');
  } catch (err) {
    console.warn('geoip-lite module not available, fallback to unknown:', err.message);
    return null;
  }
})();

let countryNames = {};
try {
  countryNames = require('./country_names.json');
} catch (_) {
  countryNames = {};
}

const UNKNOWN_INFO = Object.freeze({
  countryCode: 'UNKNOWN',
  countryName: 'Unknown',
  flagUrl: '/assets/images/flags/4x3/unknown.svg',
  isUnknown: true
});

function isPrivateIp(ip) {
  if (!ip || typeof ip !== 'string') return true;
  const clean = ip.replace(/^.*:/, '').trim();
  if (!clean || clean === '127.0.0.1' || clean === 'localhost') return true;
  const parts = clean.split('.').map(Number);
  if (parts.length !== 4 || parts.some((p) => Number.isNaN(p) || p < 0 || p > 255)) {
    return true;
  }
  // 10.0.0.0 – 10.255.255.255
  if (parts[0] === 10) return true;
  // 172.16.0.0 – 172.31.255.255
  if (parts[0] === 172 && parts[1] >= 16 && parts[1] <= 31) return true;
  // 192.168.0.0 – 192.168.255.255
  if (parts[0] === 192 && parts[1] === 168) return true;
  // 169.254.0.0 – 169.254.255.255 (link-local)
  if (parts[0] === 169 && parts[1] === 254) return true;
  // 100.64.0.0/10 (carrier-grade NAT)
  if (parts[0] === 100 && parts[1] >= 64 && parts[1] <= 127) return true;
  // 0.0.0.0/8
  if (parts[0] === 0) return true;
  return false;
}

// The game records the gateway container address. When that value is private
// and the website saw a public address for the same account, show the public one.
function displayGameIp(gameIp, siteIp) {
  const game = String(gameIp || '').replace(/^::ffff:/i, '').trim();
  if (!game) return '';
  if (!isPrivateIp(game)) return game;
  const site = String(siteIp || '').replace(/^::ffff:/i, '').trim();
  if (site && !isPrivateIp(site)) return site;
  return game;
}

function getCountryInfo(ip) {
  if (!ip || isPrivateIp(ip) || !geoipLite) {
    return { ...UNKNOWN_INFO };
  }

  try {
    const clean = ip.replace(/^.*:/, '').trim();
    const geo = geoipLite.lookup(clean);
    if (!geo || !geo.country) {
      return { ...UNKNOWN_INFO };
    }

    const code = String(geo.country).trim().toUpperCase();

    // RULE: Russia must be treated as unknown
    if (code === 'RU' || code === 'RUS') {
      return { ...UNKNOWN_INFO };
    }

    const lowerCode = code.toLowerCase();
    const name = countryNames[lowerCode] || code;

    return {
      countryCode: code,
      countryName: name,
      flagUrl: `/assets/images/flags/4x3/${lowerCode}.svg`,
      isUnknown: false
    };
  } catch (err) {
    return { ...UNKNOWN_INFO };
  }
}

module.exports = {
  getCountryInfo,
  isPrivateIp,
  displayGameIp,
  UNKNOWN_INFO
};
