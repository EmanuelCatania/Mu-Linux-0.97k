// GameMaster.txt levels match Command.txt: 1 GameMaster, 2+ Administrator.
function parseGameMaster(content) {
  const entries = [];
  const lines = String(content || '').split(/\r?\n/);
  for (const rawLine of lines) {
    const line = rawLine.split('//')[0].trim();
    if (!line) continue;
    if (line.toLowerCase() === 'end') break;
    const match = line.match(/"([^"]+)"\s+"([^"]+)"\s+(\d+)/);
    if (!match) continue;
    entries.push({
      account: match[1],
      name: match[2],
      level: Number(match[3])
    });
  }
  return entries;
}

function staffFromLevel(level) {
  const value = Number(level);
  if (value === 1) return { role: 'gm', label: 'GM' };
  if (value >= 2) return { role: 'admin', label: 'Admin' };
  return null;
}

function buildStaffIndex(entries) {
  const byName = new Map();
  for (const entry of entries || []) {
    const staff = staffFromLevel(entry.level);
    const name = String(entry.name || '').trim();
    if (!staff || !name) continue;
    const key = name.toLowerCase();
    const prev = byName.get(key);
    const level = Number(entry.level);
    if (prev && prev.level > level) continue;
    byName.set(key, {
      account: String(entry.account || '').trim(),
      name,
      level,
      role: staff.role,
      label: staff.label
    });
  }
  return {
    byName,
    names: [...byName.values()].map((entry) => entry.name)
  };
}

function lookupStaff(index, name) {
  if (!index || !name) return null;
  return index.byName.get(String(name).trim().toLowerCase()) || null;
}

module.exports = { parseGameMaster, staffFromLevel, buildStaffIndex, lookupStaff };
