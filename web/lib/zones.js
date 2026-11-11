// Zonas de cada mapa en los dos formatos que entiende el GameServer (Fase 5,
// datos por mapa):
//
//   Data/Custom/CustomSafeZone.txt      rectángulos donde nadie recibe daño
//   Data/Custom/CustomPkFree.txt        rectángulos donde matar no te vuelve PK
//   Data/Custom/CustomNpcMove.txt       NPCs que llevan a otro lugar
//   Data/Maps/NN-Nombre/Zones.toml      las tres listas de cada mapa
//
// El editor del panel trabaja sobre el texto de los .txt (una lista por vez).
// Cuando el server usa los TOML, el panel los convierte a ese texto para
// mostrarlos y de vuelta al guardar. Las conversiones son las mismas que hace
// mu-data (Common/MapZones.cpp en el server); los dos lados tienen que dar el
// mismo archivo.

const TOML = require('smol-toml');
const { MAPS_DIR, folderName } = require('./spawns');

const ZONE_FILE = 'Zones.toml';
const KINDS = ['safezone', 'pkfree', 'npcmove'];
const KEY_OF = { safezone: 'safe', pkfree: 'pk_free', npcmove: 'npc_move' };
const MOVE_COLUMNS = ['npc', 'map', 'x', 'y', 'moveMap', 'moveX', 'moveY', 'minLevel', 'maxLevel', 'minReset', 'maxReset', 'account', 'pkMove'];
const MOVE_KEYS = ['npc', 'at', 'to', 'level', 'reset', 'account', 'pk_move'];

class ZoneError extends Error {}

const empty = () => ({ safezone: [], pkfree: [], npcmove: [] });

// ---------------------------------------------------------------------------
// Los .txt de Data/Custom. -1 = "*".

function legacyRows(text, columns, header, name) {
  const rows = [];
  const lines = String(text || '').split(/\r?\n/);
  for (let n = 0; n < lines.length; n++) {
    const where = `${name}:${n + 1}`;
    const slash = lines[n].indexOf('//');
    const data = slash >= 0 ? lines[n].slice(0, slash) : lines[n];
    const comment = slash >= 0 ? lines[n].slice(slash + 2).trim() : '';
    const tokens = data.split(/\s+/).filter(Boolean);
    if (!tokens.length) continue;
    if (tokens[0] === 'end') break;
    if (tokens.length !== columns) throw new ZoneError(`${where}: se esperan ${columns} columnas (${header})`);
    const values = tokens.map((t) => {
      if (!(t === '*' || /^\d+$/.test(t) || t === '-1')) throw new ZoneError(`${where}: '${t}' no es un número válido (* o >= 0)`);
      return t === '*' ? -1 : Number(t);
    });
    rows.push({ values, comment, where });
  }
  return rows;
}

function parseLegacy(kind, text) {
  if (kind === 'npcmove') {
    return legacyRows(text, 13, 'Npc NpcMap NpcX NpcY MoveMap MoveX MoveY MinLvl MaxLvl MinRes MaxRes VIPLvl PKMove', 'CustomNpcMove.txt').map(({ values, comment, where }) => {
      const d = { comment };
      MOVE_COLUMNS.forEach((c, i) => { d[c] = values[i]; });
      const coords = [d.x, d.y, d.moveX, d.moveY, d.map, d.moveMap];
      if (d.npc < 0 || coords.some((v) => v < 0 || v > 255) || d.account < 0 || (d.pkMove !== 0 && d.pkMove !== 1)) {
        throw new ZoneError(`${where}: valor fuera de rango (mapas y coordenadas de 0 a 255, VIPLvl >= 0, PKMove 0 o 1)`);
      }
      return d;
    });
  }
  const name = kind === 'safezone' ? 'CustomSafeZone.txt' : 'CustomPkFree.txt';
  return legacyRows(text, 5, 'Map X Y TX TY', name).map(({ values, comment, where }) => {
    if (values.some((v) => v < 0 || v > 255)) throw new ZoneError(`${where}: el mapa y las coordenadas van de 0 a 255`);
    const [map, x, y, tx, ty] = values;
    return { map, x, y, tx, ty, comment };
  });
}

function buildLegacy(kind, list) {
  const star = (v) => (v < 0 ? '*' : String(v));
  const suffix = (c) => (c ? `\t//${c}` : '');
  const lines = [];
  if (kind === 'npcmove') {
    lines.push('//Npc\tNpcMap\tNpcX\tNpcY\tMoveMap\tMoveX\tMoveY\tMinLvl\tMaxLvl\tMinRes\tMaxRes\tVIPLvl\tPKMove\tComment');
    for (const d of list) lines.push(MOVE_COLUMNS.map((c) => star(d[c])).join('\t') + suffix(d.comment));
  } else {
    lines.push('//Map\tX\tY\tTX\tTY\tComment');
    for (const r of list) lines.push([r.map, r.x, r.y, r.tx, r.ty].join('\t') + suffix(r.comment));
  }
  lines.push('end');
  return lines.join('\n') + '\n';
}

// ---------------------------------------------------------------------------
// Zones.toml

// Mismo texto que MapZones_WriteToml.
function fileText(map, mapName, data) {
  let out = `# ${mapName} (mapa ${map}): zonas.\n`
    + '#\n'
    + '# safe      [x, y, tx, ty] donde nadie recibe daño\n'
    + '# pk_free   [x, y, tx, ty] donde matar no te vuelve PK\n'
    + '# npc_move  hablar con el NPC `npc` parado en `at` = [x, y] lleva a\n'
    + '#           `to` = [mapa, x, y]. Opcionales: level y reset = [mínimo,\n'
    + '#           máximo] (-1 = sin límite), account (nivel de cuenta mínimo)\n'
    + '#           y pk_move (si pueden usarlo los PK; sin la clave, no)\n';
  const comment = (c) => (c ? ` # ${c}` : '');
  for (const kind of ['safezone', 'pkfree']) {
    const rects = data[kind].filter((r) => r.map === map);
    if (!rects.length) continue;
    out += `\n${KEY_OF[kind]} = [\n`;
    for (const r of rects) out += `  [${r.x}, ${r.y}, ${r.tx}, ${r.ty}],${comment(r.comment)}\n`;
    out += ']\n';
  }
  const moves = data.npcmove.filter((d) => d.map === map);
  if (moves.length) {
    out += '\nnpc_move = [\n';
    for (const d of moves) {
      let line = `  { npc = ${d.npc}, at = [${d.x}, ${d.y}], to = [${d.moveMap}, ${d.moveX}, ${d.moveY}]`;
      if (d.minLevel !== -1 || d.maxLevel !== -1) line += `, level = [${d.minLevel}, ${d.maxLevel}]`;
      if (d.minReset !== -1 || d.maxReset !== -1) line += `, reset = [${d.minReset}, ${d.maxReset}]`;
      if (d.account !== 0) line += `, account = ${d.account}`;
      if (d.pkMove !== 0) line += ', pk_move = true';
      out += `${line} },${comment(d.comment)}\n`;
    }
    out += ']\n';
  }
  return out;
}

const intIn = (v, min, max) => typeof v === 'number' && Number.isInteger(v) && v >= min && v <= max;

function intList(v, count, min, max) {
  return Array.isArray(v) && v.length === count && v.every((x) => intIn(x, min, max));
}

// El `# ...` del final de cada elemento no lo devuelve el parser de TOML: se
// recupera de las líneas, siguiendo en qué lista está cada una.
function lineComments(content) {
  const out = { safe: [], pk_free: [], npc_move: [] };
  let current = null;
  for (const line of String(content || '').split(/\r?\n/)) {
    const open = /^\s*(safe|pk_free|npc_move)\s*=\s*\[\s*$/.exec(line);
    if (open) { current = open[1]; continue; }
    if (/^\s*\]\s*$/.test(line)) { current = null; continue; }
    if (current && /^\s*[[{]/.test(line)) {
      const m = /[\]}]\s*,?\s*#\s?(.*)$/.exec(line);
      out[current].push(m ? m[1].trim() : '');
    }
  }
  return out;
}

function parseTomlFile(path, content, map) {
  let doc;
  try {
    doc = TOML.parse(String(content || ''));
  } catch (err) {
    throw new ZoneError(`${path}: ${err.message}`);
  }
  for (const key of Object.keys(doc)) {
    if (!['safe', 'pk_free', 'npc_move'].includes(key)) throw new ZoneError(`${path}: sección desconocida '${key}' (valen safe, pk_free y npc_move)`);
  }
  const comments = lineComments(content);
  const data = empty();
  for (const kind of ['safezone', 'pkfree']) {
    const key = KEY_OF[kind];
    if (doc[key] === undefined) continue;
    if (!Array.isArray(doc[key])) throw new ZoneError(`${path}: '${key}' es una lista de rectángulos [x, y, tx, ty]`);
    doc[key].forEach((r, n) => {
      if (!intList(r, 4, 0, 255)) throw new ZoneError(`${path}: ${key}[${n}]: cada elemento de '${key}' es [x, y, tx, ty] con valores entre 0 y 255`);
      data[kind].push({ map, x: r[0], y: r[1], tx: r[2], ty: r[3], comment: comments[key][n] || '' });
    });
  }
  if (doc.npc_move !== undefined) {
    if (!Array.isArray(doc.npc_move)) throw new ZoneError(`${path}: 'npc_move' tiene que ser una lista`);
    doc.npc_move.forEach((e, n) => {
      const where = `${path}: npc_move[${n}]`;
      if (!e || typeof e !== 'object' || Array.isArray(e)) throw new ZoneError(`${where}: cada elemento de 'npc_move' es una tabla { npc = ..., at = [x, y], to = [mapa, x, y], ... }`);
      for (const key of Object.keys(e)) {
        if (!MOVE_KEYS.includes(key)) throw new ZoneError(`${where}: campo desconocido '${key}'`);
      }
      if (e.npc === undefined) throw new ZoneError(`${where}: falta 'npc' (el número de Monster.txt)`);
      if (!intIn(e.npc, 0, 65535)) throw new ZoneError(`${where}: 'npc' tiene que ser un entero entre 0 y 65535`);
      if (!intList(e.at, 2, 0, 255)) throw new ZoneError(`${where}: 'at' es una lista de 2 enteros entre 0 y 255`);
      if (!intList(e.to, 3, 0, 255)) throw new ZoneError(`${where}: 'to' es una lista de 3 enteros entre 0 y 255`);
      if (e.level !== undefined && !intList(e.level, 2, -1, 1000)) throw new ZoneError(`${where}: 'level' es una lista de 2 enteros entre -1 y 1000`);
      if (e.reset !== undefined && !intList(e.reset, 2, -1, 1000000)) throw new ZoneError(`${where}: 'reset' es una lista de 2 enteros entre -1 y 1000000`);
      if (e.account !== undefined && !intIn(e.account, 0, 255)) throw new ZoneError(`${where}: 'account' tiene que ser un entero entre 0 y 255`);
      if (e.pk_move !== undefined && typeof e.pk_move !== 'boolean') throw new ZoneError(`${where}: 'pk_move' tiene que ser true o false`);
      const [minLevel, maxLevel] = e.level || [-1, -1];
      const [minReset, maxReset] = e.reset || [-1, -1];
      data.npcmove.push({
        npc: e.npc, map, x: e.at[0], y: e.at[1], moveMap: e.to[0], moveX: e.to[1], moveY: e.to[2],
        minLevel, maxLevel, minReset, maxReset, account: e.account ?? 0, pkMove: e.pk_move ? 1 : 0,
        comment: comments.npc_move[n] || ''
      });
    });
  }
  return data;
}

// files = [{ path, content, map }] -> { safezone, pkfree, npcmove }, mapa por
// mapa y en el orden de cada archivo, como en el server.
function parseToml(files) {
  const all = empty();
  for (const f of [...files].sort((a, b) => a.map - b.map)) {
    const data = parseTomlFile(f.path, f.content, f.map);
    for (const kind of KINDS) all[kind].push(...data[kind]);
  }
  return all;
}

// `folders` = carpetas que ya existen por número de mapa (se respeta su nombre);
// para los mapas nuevos el nombre sale de `mapName`.
function buildToml(data, { mapName, folders = new Map() } = {}) {
  const nameOf = (map) => (mapName && mapName(map)) || `Map${map}`;
  const maps = new Set(KINDS.flatMap((k) => data[k].map((x) => x.map)));
  const files = {};
  for (const map of [...maps].sort((a, b) => a - b)) {
    files[`${MAPS_DIR}/${folders.get(map) || folderName(map, nameOf(map))}/${ZONE_FILE}`] = fileText(map, nameOf(map), data);
  }
  return files;
}

module.exports = {
  ZONE_FILE,
  KINDS,
  ZoneError,
  parseLegacy,
  buildLegacy,
  parseToml,
  buildToml
};
