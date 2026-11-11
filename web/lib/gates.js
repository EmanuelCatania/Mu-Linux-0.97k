// Gates en los dos formatos que entiende el GameServer (Fase 5, datos por
// mapa):
//
//   Data/Move/Gate.txt                  el de siempre, todas juntas
//   Data/Maps/NN-Nombre/Gates.toml      las de cada mapa
//
// El editor del panel trabaja sobre el texto del .txt. Cuando el server usa los
// TOML, el panel los convierte a ese texto para mostrarlos y de vuelta al
// guardar. Las conversiones son las mismas que hace mu-data
// (Common/MapGates.cpp en el server); los dos lados tienen que dar el mismo
// archivo.

const TOML = require('smol-toml');
const { MAPS_DIR, folderName } = require('./spawns');

const GATE_FILE = 'Gates.toml';
const TYPE_NAMES = ['destination', 'entrance', 'exit'];
const ENTRY_KEYS = ['index', 'type', 'area', 'target', 'dir', 'level', 'reset', 'account'];
const COLUMNS = ['index', 'flag', 'map', 'x', 'y', 'tx', 'ty', 'target', 'dir', 'minLevel', 'maxLevel', 'minReset', 'maxReset', 'account'];

class GateError extends Error {}

function checkUnique(gates) {
  const sorted = gates.map((g, order) => ({ g, order })).sort((a, b) => (a.g.index - b.g.index) || (a.order - b.order)).map((x) => x.g);
  for (let n = 1; n < sorted.length; n++) {
    if (sorted[n].index === sorted[n - 1].index) {
      throw new GateError(`${sorted[n].source}: la gate ${sorted[n].index} ya está en ${sorted[n - 1].source}`);
    }
  }
  return sorted;
}

// ---------------------------------------------------------------------------
// Gate.txt. -1 = "*".

function parseLegacy(text) {
  const gates = [];
  const lines = String(text || '').split(/\r?\n/);
  for (let n = 0; n < lines.length; n++) {
    const where = `Gate.txt:${n + 1}`;
    const tokens = lines[n].split('//')[0].split(/\s+/).filter(Boolean);
    if (!tokens.length) continue;
    if (tokens[0] === 'end') break;
    if (tokens.length !== 14) {
      throw new GateError(`${where}: se esperan 14 columnas (Index Flag Map X Y TX TY Target Dir MinLvl MaxLvl MinRes MaxRes AccountLvl)`);
    }
    const g = { source: where };
    tokens.forEach((t, i) => {
      if (t !== '*' && !/^-?\d+$/.test(t)) throw new GateError(`${where}: '${t}' no es un número`);
      g[COLUMNS[i]] = t === '*' ? -1 : Number(t);
    });
    let why = null;
    if (g.index < 0 || g.index > 65535) why = 'el número de gate va de 0 a 65535';
    else if (g.flag < 0 || g.flag > 2) why = 'Flag vale 0, 1 o 2';
    else if (g.map < 0 || g.map > 255) why = 'el mapa va de 0 a 255';
    else if ([g.x, g.y, g.tx, g.ty].some((v) => v < 0 || v > 255)) why = 'las coordenadas van de 0 a 255';
    else if (g.target < 0 || g.dir < -1 || g.dir > 255 || g.account < 0) why = 'Target >= 0, Dir de 0 a 255 y AccountLvl >= 0';
    else if ([g.minLevel, g.maxLevel, g.minReset, g.maxReset].some((v) => v < -1)) why = 'los niveles y resets van de 0 en adelante (o *)';
    if (why) throw new GateError(`${where}: ${why}`);
    gates.push(g);
  }
  return checkUnique(gates);
}

function buildLegacy(gates) {
  const star = (v) => (v < 0 ? '*' : String(v));
  const lines = ['//Index\tFlag\tMap\tX\tY\tTX\tTY\tTarget\tDir\tMinLvl\tMaxLvl\tMinRes\tMaxRes\tAccountLvl'];
  for (const g of [...gates].sort((a, b) => a.index - b.index)) {
    lines.push([g.index, g.flag, g.map, g.x, g.y, g.tx, g.ty, g.target, g.dir, g.minLevel, g.maxLevel, star(g.minReset), star(g.maxReset), g.account].join('\t'));
  }
  lines.push('end');
  return lines.join('\n') + '\n';
}

// ---------------------------------------------------------------------------
// Gates.toml

// Mismo texto que MapGates_WriteToml.
function fileText(map, mapName, gates) {
  let out = `# ${mapName} (mapa ${map}): gates.\n`
    + '#\n'
    + '# index    número de la gate. No se cambia: lo usan el cliente (Gate.bmd),\n'
    + '#          Move.txt y el dead_gate de los Map.toml\n'
    + '# type     "entrance" (pisarla lleva a `target`), "exit" (donde aparece\n'
    + '#          quien llega) o "destination" (sólo destino: /move, revivir)\n'
    + '# area     [x, y, tx, ty]\n'
    + '# target   gate a la que lleva una entrada\n'
    + '# dir      hacia dónde mira el personaje al llegar (0 a 7)\n'
    + '# level    [mínimo, máximo] de nivel para usarla; -1 = sin límite\n'
    + '# reset    [mínimo, máximo] de resets; sin la clave, sin límite\n'
    + '# account  nivel de cuenta mínimo; sin la clave, 0\n'
    + '\ngate = [\n';
  for (const g of gates) {
    out += `  { index = ${g.index}, type = "${TYPE_NAMES[g.flag]}", area = [${g.x}, ${g.y}, ${g.tx}, ${g.ty}]`;
    if (g.target !== 0) out += `, target = ${g.target}`;
    if (g.dir !== 0) out += `, dir = ${g.dir}`;
    if (g.minLevel !== -1 || g.maxLevel !== -1) out += `, level = [${g.minLevel}, ${g.maxLevel}]`;
    if (g.minReset !== -1 || g.maxReset !== -1) out += `, reset = [${g.minReset}, ${g.maxReset}]`;
    if (g.account !== 0) out += `, account = ${g.account}`;
    out += ' },\n';
  }
  return `${out}]\n`;
}

const intIn = (v, min, max) => typeof v === 'number' && Number.isInteger(v) && v >= min && v <= max;

function parseTomlFile(path, content, map) {
  let doc;
  try {
    doc = TOML.parse(String(content || ''));
  } catch (err) {
    throw new GateError(`${path}: ${err.message}`);
  }
  for (const key of Object.keys(doc)) {
    if (key !== 'gate') throw new GateError(`${path}: sección desconocida '${key}' (vale gate)`);
  }
  if (doc.gate === undefined) return [];
  if (!Array.isArray(doc.gate)) throw new GateError(`${path}: 'gate' tiene que ser una lista`);
  return doc.gate.map((entry, n) => {
    const where = `${path}: gate[${n}]`;
    if (!entry || typeof entry !== 'object' || Array.isArray(entry)) {
      throw new GateError(`${where}: cada elemento de 'gate' es una tabla { index = ..., area = [x, y, tx, ty], ... }`);
    }
    for (const key of Object.keys(entry)) {
      if (!ENTRY_KEYS.includes(key)) throw new GateError(`${where}: campo desconocido '${key}'`);
    }
    const int = (key, min, max, def) => {
      if (entry[key] === undefined) return def;
      if (!intIn(entry[key], min, max)) throw new GateError(`${where}: '${key}' tiene que ser un entero entre ${min} y ${max}`);
      return entry[key];
    };
    const list = (key, count, min, max, def) => {
      if (entry[key] === undefined) return def;
      const v = entry[key];
      if (!Array.isArray(v) || v.length !== count || !v.every((x) => intIn(x, min, max))) {
        throw new GateError(`${where}: '${key}' es una lista de ${count} enteros entre ${min} y ${max}`);
      }
      return v;
    };
    if (entry.index === undefined) throw new GateError(`${where}: falta 'index' (el número de la gate, el mismo del cliente)`);
    const g = { map, source: where };
    g.index = int('index', 0, 65535);
    const area = list('area', 4, 0, 255);
    if (!area) throw new GateError(`${where}: 'area' es una lista de 4 enteros entre 0 y 255`);
    [g.x, g.y, g.tx, g.ty] = area;
    g.target = int('target', 0, 65535, 0);
    g.dir = int('dir', -1, 255, 0);
    g.account = int('account', 0, 255, 0);
    [g.minLevel, g.maxLevel] = list('level', 2, -1, 1000, [-1, -1]);
    [g.minReset, g.maxReset] = list('reset', 2, -1, 1000000, [-1, -1]);
    g.flag = TYPE_NAMES.indexOf(entry.type);
    if (g.flag < 0) throw new GateError(`${where}: falta 'type' (vale "destination", "entrance" o "exit")`);
    return g;
  });
}

// files = [{ path, content, map }]. Queda ordenado por número de gate; falla si
// un número se repite.
function parseToml(files) {
  const all = [];
  for (const f of [...files].sort((a, b) => a.map - b.map)) all.push(...parseTomlFile(f.path, f.content, f.map));
  return checkUnique(all);
}

// `folders` = carpetas que ya existen por número de mapa (se respeta su nombre);
// para los mapas nuevos el nombre sale de `mapName`.
function buildToml(gates, { mapName, folders = new Map() } = {}) {
  const nameOf = (map) => (mapName && mapName(map)) || `Map${map}`;
  const byMap = new Map();
  for (const g of [...gates].sort((a, b) => a.index - b.index)) {
    if (!byMap.has(g.map)) byMap.set(g.map, []);
    byMap.get(g.map).push(g);
  }
  const files = {};
  for (const map of [...byMap.keys()].sort((a, b) => a - b)) {
    files[`${MAPS_DIR}/${folders.get(map) || folderName(map, nameOf(map))}/${GATE_FILE}`] = fileText(map, nameOf(map), byMap.get(map));
  }
  return files;
}

module.exports = {
  GATE_FILE,
  GateError,
  parseLegacy,
  buildLegacy,
  parseToml,
  buildToml
};
