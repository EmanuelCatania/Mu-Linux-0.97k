// Drops especiales en los dos formatos que entiende el GameServer (Fase 5,
// datos por mapa):
//
//   Data/Item/ItemDrop.txt              el de siempre, con el mapa como columna
//   Data/Items/Drops.toml               los que caen en cualquier mapa
//   Data/Maps/NN-Nombre/Drops.toml      los que caen sólo en ese mapa
//
// El editor del panel trabaja sobre el texto del .txt. Cuando el server usa los
// TOML, el panel los convierte a ese texto para mostrarlos y de vuelta al
// guardar. Las conversiones son las mismas que hace mu-data
// (Common/ItemDrops.cpp en el server); los dos lados tienen que dar el mismo
// archivo.

const TOML = require('smol-toml');
const { MAPS_DIR, folderName } = require('./spawns');

const DROP_FILE = 'Drops.toml';
const GLOBAL_FILE = 'Data/Items/Drops.toml';
const OPTION_NAMES = ['level', 'skill', 'luck', 'add', 'exc'];
const OPTION_FIELDS = ['levelOption', 'skillOption', 'luckOption', 'addOption', 'excOption'];
const ENTRY_KEYS = ['item', 'level', 'grade', 'options', 'monster', 'monster_level', 'rate'];

class DropError extends Error {}

// ---------------------------------------------------------------------------
// ItemDrop.txt. -1 = "*".

const LEGACY_COLUMNS = ['section', 'index', 'level', 'grade', ...OPTION_FIELDS, 'map', 'monster', 'monsterLevelMin', 'monsterLevelMax', 'rate'];

function parseLegacy(text) {
  const drops = [];
  const lines = String(text || '').split(/\r?\n/);
  for (let n = 0; n < lines.length; n++) {
    const where = `ItemDrop.txt:${n + 1}`;
    const slash = lines[n].indexOf('//');
    const data = slash >= 0 ? lines[n].slice(0, slash) : lines[n];
    const comment = slash >= 0 ? lines[n].slice(slash + 2).trim() : '';
    const tokens = data.replace(/,/g, ' ').split(/\s+/).filter(Boolean);
    if (!tokens.length) continue;
    if (tokens[0] === 'end') break;
    if (tokens.length !== 14) {
      throw new DropError(`${where}: se esperan 14 columnas (Sección,Índice Level Grade Level Skill Luck Option Exe MapNum Monster MonsterLvlMin MonsterLvlMax Rate)`);
    }
    const d = { comment };
    tokens.forEach((t, i) => {
      if (t !== '*' && !/^-?\d+$/.test(t)) throw new DropError(`${where}: '${t}' no es un número`);
      d[LEGACY_COLUMNS[i]] = t === '*' ? -1 : Number(t);
    });
    let why = null;
    if (d.section < 0 || d.section > 15 || d.index < 0 || d.index > 511) why = 'el item va de 0,0 a 15,511';
    else if (d.level < 0 || d.level > 15) why = 'el nivel del item va de 0 a 15';
    else if (d.grade < 0 || d.grade > 255) why = 'Grade va de 0 a 255';
    else if (d.map < -1 || d.map > 255 || d.monster < -1 || d.rate < -1 || d.rate > 1000000 || d.monsterLevelMin < -1 || d.monsterLevelMax < -1) {
      why = 'valor fuera de rango (MapNum 0-255, Rate -1 a 1000000, el resto * o >= 0)';
    } else if (OPTION_FIELDS.some((f) => d[f] < -1 || d[f] > 255)) why = 'los índices de ItemOptionRate van de 0 a 255 (o *)';
    if (why) throw new DropError(`${where}: ${why}`);
    drops.push(d);
  }
  return drops;
}

function buildLegacy(drops) {
  const star = (v) => (v < 0 ? '*' : String(v));
  const lines = [
    '// ItemGrade works when ExcellentRate = 0',
    '// Level, Skill, Luck, Option and Exe',
    '// Uses ItemOptionRate Indexes',
    '',
    '//Index\tLevel\tGrade\tLevel\tSkill\tLuck\tOption\tExe\tMapNum\tMonster\tMonsterLvlMin\tMonsterLvlMax\tRate\t\tComment'
  ];
  for (const d of drops) {
    const cols = [`${d.section},${String(d.index).padStart(3, '0')}`, d.level, d.grade, ...OPTION_FIELDS.map((f) => star(d[f])),
      star(d.map), star(d.monster), star(d.monsterLevelMin), star(d.monsterLevelMax), d.rate];
    lines.push(cols.join('\t') + (d.comment ? `\t\t//${d.comment}` : ''));
  }
  lines.push('end');
  return lines.join('\n') + '\n';
}

// ---------------------------------------------------------------------------
// Drops.toml

// Mismo texto que ItemDrops_WriteToml.
function entryLine(d) {
  let out = `  { item = [${d.section}, ${d.index}]`;
  if (d.level !== 0) out += `, level = ${d.level}`;
  if (d.grade !== 0) out += `, grade = ${d.grade}`;
  const options = OPTION_NAMES.map((name, i) => [name, d[OPTION_FIELDS[i]]]).filter(([, v]) => v >= 0);
  if (options.length) out += `, options = { ${options.map(([k, v]) => `${k} = ${v}`).join(', ')} }`;
  if (d.monster >= 0) out += `, monster = ${d.monster}`;
  if (d.monsterLevelMin >= 0 || d.monsterLevelMax >= 0) out += `, monster_level = [${d.monsterLevelMin}, ${d.monsterLevelMax}]`;
  out += `, rate = ${d.rate} },`;
  if (d.comment) out += ` # ${d.comment}`;
  return `${out}\n`;
}

function fileText(map, mapName, drops) {
  let out = map < 0
    ? '# Drops especiales que caen en cualquier mapa. Los de un solo mapa van en\n# Maps/NN-Nombre/Drops.toml.\n'
    : `# ${mapName} (mapa ${map}): drops especiales que caen sólo en este mapa.\n`;
  out += '#\n'
    + '# item           [sección, índice] del item (Item.txt)\n'
    + '# level          nivel del item (+N); sin la clave, 0\n'
    + '# grade          opciones excelentes fijas, si `exc` no da ninguna\n'
    + '# options        índices de ItemOptionRate.txt para level, skill, luck,\n'
    + '#                add y exc; sin la clave, esa opción no sale\n'
    + '# monster        sólo ese monstruo (Monster.txt); sin la clave, cualquiera\n'
    + '# monster_level  [mínimo, máximo] del nivel del monstruo; -1 = sin límite\n'
    + '# rate           chance de 1.000.000 por monstruo muerto; -1 = siempre\n'
    + '#\n'
    + '# Se prueban antes que el drop normal del monstruo (un item al azar de\n'
    + '# nivel cercano al suyo): si cae uno de estos, el monstruo no tira nada más.\n'
    + '\ndrop = [\n';
  for (const d of drops) out += entryLine(d);
  return `${out}]\n`;
}

function intIn(value, min, max) {
  return typeof value === 'number' && Number.isInteger(value) && value >= min && value <= max;
}

function parseTomlFile(path, content, map) {
  let doc;
  try {
    doc = TOML.parse(String(content || ''));
  } catch (err) {
    throw new DropError(`${path}: ${err.message}`);
  }
  for (const key of Object.keys(doc)) {
    if (key !== 'drop') throw new DropError(`${path}: sección desconocida '${key}' (vale drop)`);
  }
  if (doc.drop === undefined) return [];
  if (!Array.isArray(doc.drop)) throw new DropError(`${path}: 'drop' tiene que ser una lista`);
  return doc.drop.map((entry, n) => {
    const where = `${path}: drop[${n}]`;
    if (!entry || typeof entry !== 'object' || Array.isArray(entry)) {
      throw new DropError(`${where}: cada elemento de 'drop' es una tabla { item = [sección, índice], ... }`);
    }
    for (const key of Object.keys(entry)) {
      if (!ENTRY_KEYS.includes(key)) throw new DropError(`${where}: campo desconocido '${key}'`);
    }
    const d = { map, comment: '', level: 0, grade: 0, monster: -1, monsterLevelMin: -1, monsterLevelMax: -1 };
    OPTION_FIELDS.forEach((f) => { d[f] = -1; });
    const pair = (key, min, max) => {
      const v = entry[key];
      if (!Array.isArray(v) || v.length !== 2 || !intIn(v[0], min, max) || !intIn(v[1], min, max)) {
        throw new DropError(`${where}: '${key}' es [a, b] con valores entre ${min} y ${max}`);
      }
      return v;
    };
    const int = (key, min, max) => {
      if (entry[key] === undefined) return undefined;
      if (!intIn(entry[key], min, max)) throw new DropError(`${where}: '${key}' tiene que ser un entero entre ${min} y ${max}`);
      return entry[key];
    };
    if (entry.item === undefined) throw new DropError(`${where}: falta 'item'`);
    [d.section, d.index] = pair('item', 0, 511);
    if (d.section > 15) throw new DropError(`${where}: la sección del item va de 0 a 15`);
    d.level = int('level', 0, 15) ?? 0;
    d.grade = int('grade', 0, 255) ?? 0;
    d.monster = int('monster', 0, 65535) ?? -1;
    if (entry.monster_level !== undefined) [d.monsterLevelMin, d.monsterLevelMax] = pair('monster_level', -1, 1000);
    if (entry.rate === undefined) throw new DropError(`${where}: falta 'rate' (de 1.000.000; -1 = siempre)`);
    d.rate = int('rate', -1, 1000000);
    if (entry.options !== undefined) {
      const o = entry.options;
      if (!o || typeof o !== 'object' || Array.isArray(o)) {
        throw new DropError(`${where}: 'options' es una tabla { level = ..., skill = ..., luck = ..., add = ..., exc = ... }`);
      }
      for (const key of Object.keys(o)) {
        const i = OPTION_NAMES.indexOf(key);
        if (i < 0) throw new DropError(`${where}: opción desconocida '${key}' (valen level, skill, luck, add y exc)`);
        if (!intIn(o[key], 0, 255)) throw new DropError(`${where}: '${key}' tiene que ser un entero entre 0 y 255`);
        d[OPTION_FIELDS[i]] = o[key];
      }
    }
    return d;
  });
}

// El comentario de cada entrada (el `# ...` del final de la línea) no lo
// devuelve el parser de TOML: se recupera de la línea del texto.
function lineComments(content) {
  return String(content || '').split(/\r?\n/)
    .filter((line) => /^\s*\{\s*item\s*=/.test(line))
    .map((line) => {
      const m = /\}\s*,?\s*#\s?(.*)$/.exec(line);
      return m ? m[1].trim() : '';
    });
}

// files = [{ path, content, map }] (map = -1 para Items/Drops.toml). Quedan
// primero los de Items/Drops.toml y después los de cada mapa, como en el server.
function parseToml(files) {
  const ordered = [...files].sort((a, b) => a.map - b.map);
  const all = [];
  for (const f of ordered) {
    const drops = parseTomlFile(f.path, f.content, f.map);
    const comments = lineComments(f.content);
    if (comments.length === drops.length) drops.forEach((d, i) => { d.comment = comments[i]; });
    all.push(...drops);
  }
  return all;
}

// `folders` = carpetas que ya existen por número de mapa (se respeta su nombre);
// para los mapas nuevos el nombre sale de `mapName`.
function buildToml(drops, { mapName, folders = new Map() } = {}) {
  const nameOf = (map) => (mapName && mapName(map)) || `Map${map}`;
  const byMap = new Map();
  for (const d of drops) {
    if (!byMap.has(d.map)) byMap.set(d.map, []);
    byMap.get(d.map).push(d);
  }
  const files = {};
  for (const map of [...byMap.keys()].sort((a, b) => a - b)) {
    const path = map < 0 ? GLOBAL_FILE : `${MAPS_DIR}/${folders.get(map) || folderName(map, nameOf(map))}/${DROP_FILE}`;
    files[path] = fileText(map, nameOf(map), byMap.get(map));
  }
  return files;
}

module.exports = {
  DROP_FILE,
  GLOBAL_FILE,
  DropError,
  parseLegacy,
  buildLegacy,
  parseToml,
  buildToml
};
