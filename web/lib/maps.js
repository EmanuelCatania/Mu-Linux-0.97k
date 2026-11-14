// Reglas de cada mapa en los dos formatos que entiende el GameServer (Fase 5,
// datos por mapa):
//
//   Data/MapManager.txt                 el de siempre, una línea por mapa
//   Data/Maps/NN-Nombre/Map.toml        lo de cada mapa, junto a sus spawns
//
// El editor del panel trabaja sobre el texto del .txt. Cuando el server usa los
// TOML, el panel los convierte a ese texto para mostrarlos y de vuelta al
// guardar. Las conversiones son las mismas que hace mu-data (Common/MapInfo.cpp
// en el server); los dos lados tienen que dar el mismo archivo.

const TOML = require('smol-toml');
const { MAPS_DIR, folderName } = require('./spawns');

const MAP_FILE = 'Map.toml';
const PK_NAMES = ['allowed', 'disabled', 'no_outlaws'];
const NUMBERS = [
  ['viewRange', 'view_range', 0, 255],
  ['experienceRate', 'experience_rate', 0, 1000000],
  ['itemDropRate', 'item_drop_rate', 0, 1000000],
  ['excItemDropRate', 'exc_item_drop_rate', 0, 1000000],
  ['deadGate', 'dead_gate', 0, 65535]
];
const KEYS = ['name', 'pk', ...NUMBERS.map((n) => n[1]), 'respawn_in_place', 'flying_dragons', 'music'];
// `music`: nombre de un mp3 de Data/Music del cliente sin la extensión; "*" (o
// sin la clave) deja el del cliente. Mismas reglas que MapInfo.cpp.
const MUSIC_RE = /^[A-Za-z0-9_-]{1,27}$/;

class MapError extends Error {}

// ---------------------------------------------------------------------------
// MapManager.txt. -1 = "*": el valor por defecto del server.

function parseLegacy(text) {
  const maps = [];
  const lines = String(text || '').split(/\r?\n/);
  for (let index = 0; index < lines.length; index++) {
    const raw = lines[index];
    const where = `MapManager.txt:${index + 1}`;
    const line = raw.split('//')[0];
    const tokens = [];
    const re = /"([^"]*)"|[^\s"]+/g;
    let m;
    while ((m = re.exec(line)) !== null) tokens.push(m[1] !== undefined ? m[1] : m[0]);
    if (!tokens.length) continue;
    if (tokens[0] === 'end') break; // lo que sigue a `end` no cuenta
    if (tokens.length !== 10) {
      throw new MapError(`${where}: se esperan 10 columnas (Index NonPK View ExperienceRate ItemDropRate ExcDropRate DeadGate SpawnInPlace FlyingDragons "Nombre")`);
    }
    const values = tokens.slice(0, 9).map((t) => {
      if (t === '*') return -1;
      if (!/^-?\d+$/.test(t)) throw new MapError(`${where}: '${t}' no es un número`);
      const v = Number(t);
      if (v < -1) throw new MapError(`${where}: '${t}' no puede ser negativo (* = el valor por defecto)`);
      return v;
    });
    const def = {
      index: values[0],
      nonPK: values[1],
      viewRange: values[2],
      experienceRate: values[3],
      itemDropRate: values[4],
      excItemDropRate: values[5],
      deadGate: values[6],
      respawnInPlace: values[7],
      flyingDragons: values[8],
      name: tokens[9]
    };
    if (def.index < 0 || def.index > 255) throw new MapError(`${where}: el número de mapa va de 0 a 255`);
    if (def.nonPK > 2) throw new MapError(`${where}: NonPK vale *, 0, 1 o 2`);
    if (!def.name || def.name.length >= 32) throw new MapError(`${where}: el nombre va de 1 a 31 caracteres`);
    if (maps.some((o) => o.index === def.index)) throw new MapError(`${where}: el mapa ${def.index} está repetido`);
    maps.push(def);
  }
  return maps.sort((a, b) => a.index - b.index);
}

function buildLegacy(maps) {
  const star = (v) => (v < 0 ? '*' : String(v));
  const lines = ['//Index\tNonPK\tView\tExperienceRate\tItemDropRate\tExcDropRate\tDeadGate\tSpawnInPlace\tFlyingDragons\tName'];
  for (const d of [...maps].sort((a, b) => a.index - b.index)) {
    lines.push([d.index, star(d.nonPK), star(d.viewRange), star(d.experienceRate), star(d.itemDropRate),
      star(d.excItemDropRate), star(d.deadGate), d.respawnInPlace ? 1 : 0, d.flyingDragons ? 1 : 0, `"${d.name}"`].join('\t'));
  }
  lines.push('end');
  return lines.join('\n') + '\n';
}

// ---------------------------------------------------------------------------
// Map.toml

function quote(text) {
  return `"${String(text).replace(/["\\]/g, (c) => `\\${c}`)}"`;
}

// Mismo texto que MapInfo_WriteToml.
function mapFileText(d) {
  let out = `# ${d.name} (mapa ${d.index}): reglas del mapa.\n`
    + '#\n'
    + '# name                nombre del mapa en los avisos del server\n'
    + '# pk                  "allowed", "disabled" o "no_outlaws"; sin pk, lo que\n'
    + '#                     diga NonPK en la configuración del server\n'
    + '# view_range          tiles que ve un jugador (sin la clave, 15)\n'
    + '# experience_rate     % de experiencia (sin la clave, 100)\n'
    + '# item_drop_rate      % de drop de items (sin la clave, 100)\n'
    + '# exc_item_drop_rate  % de items excelentes (sin la clave, 0)\n'
    + '# dead_gate           gate (Gate.txt) al que se va al morir\n'
    + '# respawn_in_place    revivir donde se murió en vez de en dead_gate\n'
    + '# flying_dragons      salen los dragones voladores del evento\n'
    + '# music               tema del cliente: nombre de un mp3 de Data/Music sin la\n'
    + '#                     extensión ("Pub" = Data/Music/Pub.mp3); "*" o sin la\n'
    + '#                     clave, el del cliente (el del juego o un mp3 con el\n'
    + '#                     nombre del mapa)\n'
    + '\n'
    + `name = ${quote(d.name)}\n`;
  if (d.nonPK >= 0 && d.nonPK <= 2) out += `pk = "${PK_NAMES[d.nonPK]}"\n`;
  for (const [field, key] of NUMBERS) {
    if (d[field] >= 0) out += `${key} = ${d[field]}\n`;
  }
  out += `respawn_in_place = ${d.respawnInPlace ? 'true' : 'false'}\n`;
  out += `flying_dragons = ${d.flyingDragons ? 'true' : 'false'}\n`;
  out += `music = ${quote(d.music || '*')}\n`;
  return out;
}

function parseTomlFile(path, content, map) {
  let doc;
  try {
    doc = TOML.parse(String(content || ''));
  } catch (err) {
    throw new MapError(`${path}: ${err.message}`);
  }
  for (const key of Object.keys(doc)) {
    if (!KEYS.includes(key)) throw new MapError(`${path}: campo desconocido '${key}'`);
  }
  const d = { index: map, nonPK: -1, respawnInPlace: 0, flyingDragons: 0, music: '' };
  if (typeof doc.name !== 'string' || !doc.name || doc.name.length >= 32) {
    throw new MapError(`${path}: falta 'name' (texto de 1 a 31 caracteres)`);
  }
  d.name = doc.name;
  if (doc.pk !== undefined) {
    d.nonPK = PK_NAMES.indexOf(doc.pk);
    if (d.nonPK < 0) {
      throw new MapError(`${path}: 'pk' vale "allowed", "disabled" o "no_outlaws" (sin 'pk', lo que diga NonPK en la configuración del server)`);
    }
  }
  for (const [field, key, min, max] of NUMBERS) {
    const v = doc[key];
    if (v === undefined) {
      d[field] = -1;
      continue;
    }
    if (!Number.isInteger(Number(v)) || typeof v === 'boolean' || typeof v === 'string' || Number(v) < min || Number(v) > max) {
      throw new MapError(`${path}: '${key}' tiene que ser un entero entre ${min} y ${max}`);
    }
    d[field] = Number(v);
  }
  for (const [field, key] of [['respawnInPlace', 'respawn_in_place'], ['flyingDragons', 'flying_dragons']]) {
    if (doc[key] === undefined) continue;
    if (typeof doc[key] !== 'boolean') throw new MapError(`${path}: '${key}' tiene que ser true o false`);
    d[field] = doc[key] ? 1 : 0;
  }
  if (doc.music !== undefined) {
    if (typeof doc.music !== 'string' || (doc.music !== '*' && !MUSIC_RE.test(doc.music))) {
      throw new MapError(`${path}: 'music' es el nombre de un mp3 de Data/Music del cliente, sin la extensión (letras, números, '_' y '-', hasta 27), o "*" para el del cliente`);
    }
    d.music = doc.music === '*' ? '' : doc.music;
  }
  return d;
}

// MapManager.txt no tiene columna de música: al guardar desde ese texto se
// conserva la que ya tenía cada mapa en su Map.toml.
function keepMusic(maps, previous) {
  const music = new Map(previous.map((d) => [d.index, d.music || '']));
  return maps.map((d) => ({ ...d, music: music.get(d.index) || '' }));
}

// files = [{ path, content, map }]
function parseToml(files) {
  return files.map((f) => parseTomlFile(f.path, f.content, f.map)).sort((a, b) => a.index - b.index);
}

// `folders` = carpetas que ya existen por número de mapa (se respeta su nombre);
// para los mapas nuevos el nombre sale del `name` del mapa.
function buildToml(maps, { folders = new Map() } = {}) {
  const files = {};
  for (const d of [...maps].sort((a, b) => a.index - b.index)) {
    const folder = folders.get(d.index) || folderName(d.index, d.name);
    files[`${MAPS_DIR}/${folder}/${MAP_FILE}`] = mapFileText(d);
  }
  return files;
}

module.exports = {
  MAP_FILE,
  keepMusic,
  MapError,
  parseLegacy,
  buildLegacy,
  parseToml,
  buildToml
};
