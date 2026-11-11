// Spawns de monstruos y NPCs en los dos formatos que entiende el GameServer
// (Fase 5, datos por mapa):
//
//   Data/Monster/MonsterSetBase.txt            el de siempre, con el mapa como columna
//   Data/Maps/NN-Nombre/Spawns.toml            lo de cada mapa
//   Data/Events/Invasions/Spawns.toml          las invasiones, que cruzan mapas
//
// El editor del panel trabaja sobre el texto del .txt. Cuando el server usa los
// TOML, el panel los convierte a ese texto para mostrarlos y de vuelta al
// guardar. Las conversiones son las mismas que hace mu-data (Common/MapSpawns.cpp
// en el server); los dos lados tienen que dar el mismo archivo.

const TOML = require('smol-toml');

const SECTION_NAMES = ['npc', 'area', 'spot', 'invasion', 'event'];
const INVASION = 3;
const isBox = (section) => section === 1 || section === 3;

const MAPS_DIR = 'Data/Maps';
const INVASIONS_FILE = 'Data/Events/Invasions/Spawns.toml';

class SpawnError extends Error {}

function folderName(map, name) {
  return `${String(map).padStart(2, '0')}-${String(name || `Map${map}`).replace(/[^A-Za-z0-9]/g, '')}`;
}

// Número de mapa del prefijo "NN-" de la carpeta; -1 si no tiene.
function folderMap(folder) {
  const match = /^(\d+)-/.exec(folder);
  return match ? Number(match[1]) : -1;
}

// ---------------------------------------------------------------------------
// MonsterSetBase.txt

function parseLegacy(text) {
  const tokens = [];
  const comments = new Map();
  String(text || '').split(/\r?\n/).forEach((raw, index) => {
    const line = index + 1;
    let body = raw;
    const comment = body.indexOf('//');
    if (comment !== -1) {
      comments.set(line, body.slice(comment + 2).trim());
      body = body.slice(0, comment);
    }
    for (const word of body.split(/\s+/).filter(Boolean)) {
      tokens.push({ text: word, line });
    }
  });

  let pos = 0;
  const number = () => {
    if (pos >= tokens.length) throw new SpawnError('El archivo termina en medio de una línea');
    const token = tokens[pos++];
    if (!/^-?\d+$/.test(token.text)) {
      throw new SpawnError(`Línea ${token.line}: se esperaba un número y hay '${token.text}'`);
    }
    return Number(token.text);
  };

  const entries = [];
  while (pos < tokens.length && tokens[pos].text !== 'end') {
    const section = number();
    if (section < 0 || section > 4) {
      throw new SpawnError(`Línea ${tokens[pos - 1].line}: sección ${section} desconocida`);
    }
    while (pos < tokens.length && tokens[pos].text !== 'end') {
      const line = tokens[pos].line;
      const e = { section, monster: number(), map: number(), dis: number(), x: number(), y: number(), tx: 0, ty: 0, dir: -1, count: 1, value: 0 };
      if (isBox(section)) {
        e.tx = number();
        e.ty = number();
      }
      e.dir = number();
      if (isBox(section)) {
        e.count = number();
        if (section === INVASION) e.value = number();
      }
      e.comment = comments.get(line) || '';
      entries.push(e);
    }
    pos += 1;
  }
  return entries;
}

// Texto del .txt, sección por sección (un bloque por sección y mapa, como los
// que trae el archivo original). `monsterName(id)` da el comentario de cada línea.
function buildLegacy(entries, monsterName) {
  const out = [];
  for (let section = 0; section < 5; section += 1) {
    const list = entries.filter((e) => e.section === section);
    if (!list.length) continue;
    out.push(String(section));
    for (const e of list) {
      const cols = [e.monster, e.map, e.dis, e.x, e.y];
      if (isBox(section)) cols.push(e.tx, e.ty);
      cols.push(e.dir);
      if (isBox(section)) {
        cols.push(e.count);
        if (section === INVASION) cols.push(e.value);
      }
      const name = e.comment || (monsterName && monsterName(e.monster)) || '';
      out.push(cols.join('\t') + (name ? `\t// ${name}` : ''));
    }
    out.push('end');
    out.push('');
  }
  return out.join('\n');
}

// ---------------------------------------------------------------------------
// TOML

function entryLine(e, withMap, mapName) {
  let line = '  { ';
  if (withMap) line += `map = ${e.map}, `;
  line += `monster = ${e.monster}`;
  if (isBox(e.section)) {
    line += `, box = [${e.x}, ${e.y}, ${e.tx}, ${e.ty}], count = ${e.count}`;
    if (e.section === INVASION) line += `, value = ${e.value}`;
  } else {
    line += `, at = [${e.x}, ${e.y}]`;
  }
  if (e.dir !== -1) line += `, dir = ${e.dir}`;
  if (e.dis !== 0) line += `, dis = ${e.dis}`;
  line += ' },';
  const comment = [e.comment, mapName].filter(Boolean).join(' - ');
  return comment ? `${line} # ${comment}` : line;
}

function mapFileText(map, mapName, entries) {
  const lines = [
    `# ${mapName} (mapa ${map}): monstruos y NPCs.`,
    '#',
    '# npc    fijo en `at` = [x, y]',
    '# area   `count` monstruos repartidos en `box` = [x, y, tx, ty]',
    '# spot   uno, a 3 tiles o menos de `at`',
    '# event  como npc, para los eventos del mapa (Blood Castle, Devil Square)',
    '#',
    '# Los puntos de las invasiones no van acá: cruzan mapas y viven en',
    '# Events/Invasions/Spawns.toml.',
    '#',
    '# dir: 0 a 7, o -1 (o sin dir) para una al azar. dis: distancia que se',
    '# mueve el monstruo desde su lugar (0 si no se mueve).'
  ];
  for (let section = 0; section < 5; section += 1) {
    if (section === INVASION) continue;
    const list = entries.filter((e) => e.section === section);
    if (!list.length) continue;
    lines.push('', `${SECTION_NAMES[section]} = [`, ...list.map((e) => entryLine(e, false, '')), ']');
  }
  return `${lines.join('\n')}\n`;
}

function invasionsFileText(entries, mapName) {
  const lines = [
    '# Puntos de aparición de las invasiones (Death King, Red Dragon, dorados...).',
    '#',
    '# Cruzan mapas, así que no están en ninguna carpeta de Maps: cada entrada',
    '# dice en qué `map` aparece. Las invasiones en sí (horarios, jefes,',
    '# mensajes) están en Invasions.toml, en esta misma carpeta; ahí, cada',
    '# grupo elige por mapa y por `value` qué puntos de esta lista usa.',
    '#',
    '# `count` monstruos repartidos en `box` = [x, y, tx, ty]. dir: 0 a 7, o -1',
    '# (o sin dir) para una al azar. dis: distancia que se mueve el monstruo.',
    '',
    'invasion = ['
  ];
  for (const e of entries) {
    if (e.section === INVASION) lines.push(entryLine(e, true, mapName(e.map)));
  }
  lines.push(']');
  return `${lines.join('\n')}\n`;
}

// Archivos TOML para un conjunto de entradas: { ruta: contenido }.
// `folders` = carpetas que ya existen por número de mapa (se respeta su
// nombre); para los mapas nuevos el nombre sale de `mapName`.
function buildToml(entries, { mapName, folders = new Map() } = {}) {
  const nameOf = (map) => (mapName && mapName(map)) || `Map${map}`;
  const files = {};
  const maps = new Set([...folders.keys(), ...entries.filter((e) => e.section !== INVASION).map((e) => e.map)]);
  for (const map of [...maps].sort((a, b) => a - b)) {
    const folder = folders.get(map) || folderName(map, nameOf(map));
    files[`${MAPS_DIR}/${folder}/Spawns.toml`] = mapFileText(map, nameOf(map), entries.filter((e) => e.map === map && e.section !== INVASION));
  }
  if (entries.some((e) => e.section === INVASION)) {
    files[INVASIONS_FILE] = invasionsFileText(entries, nameOf);
  }
  return files;
}

function readInt(entry, key, { required = false, def = 0, min, max }, where) {
  if (!(key in entry)) {
    if (required) throw new SpawnError(`${where}: falta '${key}'`);
    return def;
  }
  const value = entry[key];
  const n = typeof value === 'bigint' ? Number(value) : value;
  if (!Number.isInteger(n) || n < min || n > max) {
    throw new SpawnError(`${where}: '${key}' tiene que ser un entero entre ${min} y ${max}`);
  }
  return n;
}

function readCoords(entry, key, count, where) {
  const value = entry[key];
  if (!Array.isArray(value) || value.length !== count) {
    throw new SpawnError(`${where}: falta '${key}' con ${count} coordenadas`);
  }
  return value.map((v) => {
    const n = typeof v === 'bigint' ? Number(v) : v;
    if (!Number.isInteger(n) || n < 0 || n > 255) throw new SpawnError(`${where}: '${key}': las coordenadas van de 0 a 255`);
    return n;
  });
}

// Lee un Spawns.toml. `map` = número de la carpeta, o null para el archivo de
// invasiones (el mapa va en cada entrada). Mismas reglas que el GameServer.
function parseTomlFile(path, content, map) {
  const invasions = map === null;
  let doc;
  try {
    doc = TOML.parse(String(content || ''));
  } catch (err) {
    throw new SpawnError(`${path}: ${err.message}`);
  }
  const entries = [];
  for (const key of Object.keys(doc)) {
    const section = SECTION_NAMES.indexOf(key);
    const allowed = section !== -1 && (invasions ? section === INVASION : section !== INVASION);
    if (!allowed) {
      if (!invasions && key === 'event_area') {
        throw new SpawnError(`${path}: los puntos de las invasiones cruzan mapas: van en Events/Invasions/Spawns.toml, con \`map\` en cada entrada`);
      }
      throw new SpawnError(`${path}: sección desconocida '${key}' (${invasions ? 'vale invasion' : 'valen npc, area, spot y event'})`);
    }
    if (!Array.isArray(doc[key])) throw new SpawnError(`${path}: '${key}' tiene que ser una lista`);
    doc[key].forEach((entry, index) => {
      const where = `${path}: ${key}[${index}]`;
      if (!entry || typeof entry !== 'object' || Array.isArray(entry)) {
        throw new SpawnError(`${where}: cada elemento es una tabla { monster = ..., ... }`);
      }
      const box = isBox(section);
      for (const field of Object.keys(entry)) {
        const ok = ['monster', 'dir', 'dis'].includes(field) || (box ? field === 'box' || field === 'count' : field === 'at')
          || (section === INVASION && field === 'value') || (invasions && field === 'map');
        if (!ok) throw new SpawnError(`${where}: campo desconocido '${field}'`);
      }
      const e = { section, comment: '', tx: 0, ty: 0, count: 1, value: 0 };
      e.map = invasions ? readInt(entry, 'map', { required: true, min: 0, max: 255 }, where) : map;
      e.monster = readInt(entry, 'monster', { required: true, min: 0, max: 65535 }, where);
      e.dir = readInt(entry, 'dir', { def: -1, min: -1, max: 7 }, where);
      e.dis = readInt(entry, 'dis', { def: 0, min: 0, max: 255 }, where);
      const coords = readCoords(entry, box ? 'box' : 'at', box ? 4 : 2, where);
      [e.x, e.y] = coords;
      if (box) {
        [, , e.tx, e.ty] = coords;
        e.count = readInt(entry, 'count', { required: true, min: 0, max: 10000 }, where);
        e.value = readInt(entry, 'value', { def: 0, min: -2147483648, max: 2147483647 }, where);
      }
      entries.push(e);
    });
  }
  return entries;
}

// Junta todos los archivos en el orden del GameServer: por sección, después por
// mapa, y dentro de cada mapa en el orden del archivo.
// `files` = [{ path, map (null para invasiones), content }].
function parseToml(files) {
  const all = [];
  for (const file of files) all.push(...parseTomlFile(file.path, file.content, file.map));
  return all
    .map((e, order) => ({ e, order }))
    .sort((a, b) => (a.e.section - b.e.section) || (a.e.map - b.e.map) || (a.order - b.order))
    .map(({ e }) => e);
}

module.exports = {
  MAPS_DIR,
  INVASIONS_FILE,
  SpawnError,
  folderName,
  folderMap,
  parseLegacy,
  buildLegacy,
  buildToml,
  parseToml
};
