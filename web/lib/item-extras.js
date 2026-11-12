'use strict';

// Datos extra de los items: Data/Items/NN-nombre.toml, un archivo por sección
// (Fase 5). Cada tabla es un item por su índice: [020] es el item 0,20.
//
// Reemplazan a CustomItem/CustomGlow/CustomBow/ItemPosition del Encoder,
// CustomWing.txt, ItemStack.txt, ItemValue.txt y los JSON de Custom/Items y
// Custom/Pets. El formato y las reglas son los de Common/ItemExtras.cpp del
// server (mu-data escribe estos archivos igual que este módulo).
//
// Al guardar se reemplaza sólo el bloque del item que cambió: el resto del
// archivo (comentarios incluidos) queda como estaba.

const { parse } = require('smol-toml');

const SECTION_NAMES = [
  'swords', 'axes', 'maces', 'spears', 'bows', 'staffs', 'shields', 'helms',
  'armors', 'pants', 'gloves', 'boots', 'wings', 'helpers', 'potions', 'scrolls'
];

const ITEMS_DIR = 'Data/Items';

const MAX_INDEX = 511;

const KEYS = ['model', 'glow', 'bow', 'wing', 'inventory', 'effects', 'tooltip', 'pet', 'stack', 'value'];

class ItemExtrasError extends Error {
  constructor(message) {
    super(message);
    this.name = 'ItemExtrasError';
    this.status = 400;
  }
}

function pad(n, width) {
  return String(n).padStart(width, '0');
}

function sectionFileName(section) {
  return `${pad(section, 2)}-${SECTION_NAMES[section] || 'items'}.toml`;
}

function sectionPath(section) {
  return `${ITEMS_DIR}/${sectionFileName(section)}`;
}

// "00-swords.toml" -> 0; -1 si no es un archivo de sección.
function fileSection(name) {
  const m = /^(\d\d)-.+\.toml$/.exec(String(name || '').split('/').pop());
  if (!m) return -1;
  const section = Number(m[1]);
  return section < SECTION_NAMES.length ? section : -1;
}

function isInt(v, min, max) {
  return Number.isInteger(v) && v >= min && v <= max;
}

function levelOk(level) {
  return level === undefined || level === '*' || isInt(level, 0, 15);
}

// Valida un item ya leído del TOML (las mismas reglas que ReadItemTable en
// Common/ItemExtras.cpp). Lo de adentro de inventory, effects, tooltip y pet lo
// valida custom-items.js con las reglas del GameServer.
function validateItem(where, item) {
  for (const key of Object.keys(item)) {
    if (!KEYS.includes(key)) throw new ItemExtrasError(`${where}: clave desconocida '${key}' (${KEYS.join(', ')})`);
  }
  if (item.model !== undefined) {
    const m = item.model;
    const hasFolder = m && m.folder !== undefined;
    if (!m || typeof m !== 'object' || typeof m.name !== 'string' || (hasFolder && typeof m.folder !== 'string') || Object.keys(m).length !== (hasFolder ? 2 : 1)) {
      throw new ItemExtrasError(`${where}: 'model' tiene que ser { name = "..." } o { folder = "...", name = "..." }`);
    }
    if ((hasFolder && m.folder.length >= 40) || m.name.length >= 28) {
      throw new ItemExtrasError(`${where}: 'model': la carpeta tiene que tener menos de 40 caracteres y el nombre menos de 28`);
    }
  }
  if (item.glow !== undefined && !(Array.isArray(item.glow) && item.glow.length === 3 && item.glow.every((v) => isInt(v, 0, 255)))) {
    throw new ItemExtrasError(`${where}: 'glow' tiene que ser [R, G, B] con valores de 0 a 255`);
  }
  if (item.bow !== undefined && item.bow !== 'bow' && item.bow !== 'crossbow') {
    throw new ItemExtrasError(`${where}: 'bow' tiene que ser "bow" o "crossbow"`);
  }
  if (item.wing !== undefined) {
    const w = item.wing;
    const pair = (p) => Array.isArray(p) && p.length === 2 && p.every((v) => isInt(v, -100000, 100000));
    if (!w || typeof w !== 'object' || Object.keys(w).some((k) => !['defense', 'inc_damage', 'dec_damage'].includes(k)) ||
        (w.defense !== undefined && !isInt(w.defense, -100000, 100000)) ||
        (w.inc_damage !== undefined && !pair(w.inc_damage)) || (w.dec_damage !== undefined && !pair(w.dec_damage))) {
      throw new ItemExtrasError(`${where}: 'wing' no es válido (defense = N, inc_damage = [A, B], dec_damage = [A, B])`);
    }
  }
  for (const key of ['inventory', 'pet']) {
    if (item[key] !== undefined && (!item[key] || typeof item[key] !== 'object' || Array.isArray(item[key]))) {
      throw new ItemExtrasError(`${where}: '${key}' tiene que ser una tabla`);
    }
  }
  for (const key of ['effects', 'tooltip']) {
    if (item[key] !== undefined && !Array.isArray(item[key])) throw new ItemExtrasError(`${where}: '${key}' tiene que ser un array`);
  }
  if (item.stack !== undefined && !isInt(item.stack, 0, 255)) {
    if (!Array.isArray(item.stack)) throw new ItemExtrasError(`${where}: 'stack' tiene que ser un número o una lista de { level, max, create }`);
    for (const s of item.stack) {
      if (!s || typeof s !== 'object' || !levelOk(s.level) || !isInt(s.max, 0, 255) ||
          Object.keys(s).some((k) => !['level', 'max', 'create'].includes(k)) ||
          (s.create !== undefined && parseItemPair(s.create) === null)) {
        throw new ItemExtrasError(`${where}: cada 'stack' necesita max (0..255), level opcional (0..15 o "*") y create opcional ("S,I")`);
      }
    }
  }
  if (item.value !== undefined) {
    if (!Array.isArray(item.value)) throw new ItemExtrasError(`${where}: 'value' tiene que ser una lista de { level, buy, sell }`);
    for (const v of item.value) {
      if (!v || typeof v !== 'object' || !levelOk(v.level) || Object.keys(v).some((k) => !['level', 'buy', 'sell'].includes(k)) ||
          (v.buy !== undefined && !isInt(v.buy, 0, 2000000000)) || (v.sell !== undefined && !isInt(v.sell, 0, 2000000000))) {
        throw new ItemExtrasError(`${where}: cada 'value' necesita level opcional (0..15 o "*") y buy/sell de 0 a 2.000.000.000`);
      }
    }
  }
}

function parseItemPair(value) {
  if (Array.isArray(value) && value.length === 2 && isInt(value[0], 0, 15) && isInt(value[1], 0, MAX_INDEX)) return [value[0], value[1]];
  const m = /^\s*(\d+)\s*,\s*(\d+)\s*$/.exec(String(value ?? ''));
  if (!m) return null;
  const s = Number(m[1]);
  const i = Number(m[2]);
  return (s <= 15 && i <= MAX_INDEX) ? [s, i] : null;
}

// Texto de un archivo de sección -> { índice: item }.
function parseSection(text, fileName = 'items.toml') {
  let root;
  try {
    // smol-toml arma objetos sin prototipo; se pasan a objetos comunes.
    root = JSON.parse(JSON.stringify(parse(String(text || ''))));
  } catch (err) {
    throw new ItemExtrasError(`${fileName}: ${err.message}`);
  }
  const items = {};
  for (const [key, value] of Object.entries(root)) {
    if (!/^\d+$/.test(key) || Number(key) > MAX_INDEX) throw new ItemExtrasError(`${fileName}: '${key}' no es un índice de item (0 a 511)`);
    if (!value || typeof value !== 'object' || Array.isArray(value)) throw new ItemExtrasError(`${fileName} [${key}]: cada item es una tabla`);
    validateItem(`${fileName} [${key}]`, value);
    const index = Number(key);
    if (items[index]) throw new ItemExtrasError(`${fileName}: el item ${index} está dos veces`);
    items[index] = value;
  }
  return items;
}

//**********************************************//
//******************* Escritura ****************//
//**********************************************//

function tomlString(text) {
  return `"${String(text).replace(/\\/g, '\\\\').replace(/"/g, '\\"')}"`;
}

function tomlKey(key) {
  return /^[A-Za-z0-9_-]+$/.test(key) ? key : tomlString(key);
}

function tomlNumber(n) {
  return Number.isInteger(n) ? String(n) : String(n);
}

function tomlInline(v) {
  if (typeof v === 'boolean') return v ? 'true' : 'false';
  if (typeof v === 'number' || typeof v === 'bigint') return tomlNumber(Number(v));
  if (typeof v === 'string') return tomlString(v);
  if (Array.isArray(v)) return `[${v.map(tomlInline).join(', ')}]`;
  if (v && typeof v === 'object') {
    const keys = Object.keys(v).sort();
    return keys.length ? `{ ${keys.map((k) => `${tomlKey(k)} = ${tomlInline(v[k])}`).join(', ')} }` : '{}';
  }
  return '""';
}

function tomlMultiline(list) {
  if (list.length <= 1) return tomlInline(list);
  return `[\n${list.map((v) => `\t${tomlInline(v)},\n`).join('')}]`;
}

function levelText(level) {
  return (level === undefined || level === '*') ? [] : [`level = ${level}`];
}

// Un item al texto de su bloque (sin comentarios), como WriteItem de
// Common/ItemExtras.cpp.
function itemBody(index, item) {
  const key = pad(index, 3);
  const lines = [`[${key}]`];
  // Sin carpeta: la del item en el cliente, Data/Item/NN-sección/III/.
  if (item.model) {
    lines.push(item.model.folder === undefined
      ? `model = { name = ${tomlString(item.model.name)} }`
      : `model = { folder = ${tomlString(item.model.folder)}, name = ${tomlString(item.model.name)} }`);
  }
  if (item.bow) lines.push(`bow = "${item.bow}"`);
  if (item.glow) lines.push(`glow = [${item.glow.join(', ')}]`);
  if (item.wing) {
    const w = item.wing;
    const inc = w.inc_damage || [0, 0];
    const dec = w.dec_damage || [0, 0];
    lines.push(`wing = { defense = ${w.defense || 0}, inc_damage = [${inc[0]}, ${inc[1]}], dec_damage = [${dec[0]}, ${dec[1]}] }`);
  }
  if (item.inventory) lines.push(`inventory = ${tomlInline(item.inventory)}`);
  if (item.tooltip) lines.push(`tooltip = ${tomlMultiline(item.tooltip)}`);
  if (item.effects) lines.push(`effects = ${tomlMultiline(item.effects)}`);
  if (item.stack !== undefined) {
    const list = Array.isArray(item.stack) ? item.stack : [{ max: item.stack }];
    if (list.length === 1 && (list[0].level === undefined || list[0].level === '*') && list[0].create === undefined) {
      lines.push(`stack = ${list[0].max}`);
    } else if (list.length) {
      lines.push('stack = [');
      for (const s of list) {
        const fields = [...levelText(s.level), `max = ${s.max}`];
        if (s.create !== undefined) {
          const [cs, ci] = parseItemPair(s.create);
          fields.push(`create = "${cs},${ci}"`);
        }
        lines.push(`\t{ ${fields.join(', ')} },`);
      }
      lines.push(']');
    }
  }
  if (item.value && item.value.length) {
    lines.push('value = [');
    for (const v of item.value) {
      const fields = [...levelText(v.level)];
      if (v.buy !== undefined) fields.push(`buy = ${v.buy}`);
      if (v.sell !== undefined) fields.push(`sell = ${v.sell}`);
      lines.push(`\t{ ${fields.join(', ')} },`);
    }
    lines.push(']');
  }
  if (item.pet) {
    lines.push('', `[${key}.pet]`);
    for (const k of Object.keys(item.pet).sort()) {
      const v = item.pet[k];
      lines.push(`${tomlKey(k)} = ${Array.isArray(v) ? tomlMultiline(v) : tomlInline(v)}`);
    }
  }
  return lines.join('\n');
}

function isEmptyItem(item) {
  return !item || KEYS.every((k) => item[k] === undefined || (Array.isArray(item[k]) && k !== 'stack' && item[k].length === 0));
}

function sectionHeader(section) {
  return [
    `# Items de la sección ${section} (${SECTION_NAMES[section] || 'items'}): lo que no está en Item/Item.txt.`,
    `# Cada tabla es un item por su índice: [020] es el item ${section},20.`,
    '# Ver Data/Items/00-swords.toml para la lista de claves.',
    ''
  ].join('\n');
}

// Líneas del archivo agrupadas por item: cada bloque arranca en los comentarios
// pegados a su [NNN] y termina antes de los del item siguiente.
function splitBlocks(text) {
  const lines = String(text || '').replace(/\r\n/g, '\n').split('\n');
  const headers = [];
  lines.forEach((line, n) => {
    const m = /^\[(\d+)\]\s*$/.exec(line.trim());
    if (m) headers.push({ index: Number(m[1]), line: n });
  });
  const blocks = [];
  headers.forEach((h, k) => {
    let start = h.line;
    while (start > 0 && lines[start - 1].trim().startsWith('#')) start--;
    const nextHeader = (k + 1 < headers.length) ? headers[k + 1].line : lines.length;
    let end = nextHeader;
    if (k + 1 < headers.length) {
      while (end - 1 > h.line && lines[end - 1].trim().startsWith('#')) end--;
    }
    blocks.push({ index: h.index, start, header: h.line, end });
  });
  return { lines, blocks };
}

// Cambia el bloque de un item (o lo agrega en orden de índice, o lo borra con
// item = null). Los comentarios de arriba del item se conservan.
function replaceItem(text, section, index, item, name = '') {
  // Se trabaja con '\n' y se devuelve con el fin de línea que tenía el archivo.
  const eol = String(text || '').includes('\r\n') ? '\r\n' : '\n';
  return replaceItemLf(String(text || '').replace(/\r\n/g, '\n'), section, index, item, name).replace(/\n/g, eol);
}

function replaceItemLf(text, section, index, item, name) {
  const { lines, blocks } = splitBlocks(text);
  const empty = isEmptyItem(item);
  const body = empty ? null : itemBody(index, item);
  const existing = blocks.find((b) => b.index === index);
  if (!String(text || '').trim()) {
    if (empty) return '';
    return `${sectionHeader(section)}\n${name ? `# ${name}\n` : ''}${body}\n`;
  }
  let out;
  if (existing) {
    const comments = lines.slice(existing.start, existing.header);
    let tail = lines.slice(existing.end);
    const replacement = empty ? [] : [...comments, ...body.split('\n'), ''];
    // Sin el item se va también la línea en blanco que lo separaba.
    if (empty) while (tail.length && tail[0].trim() === '') tail = tail.slice(1);
    out = [...lines.slice(0, existing.start), ...replacement, ...trimLeadingBlank(tail, replacement.length > 0)];
  } else {
    if (empty) return text;
    const after = blocks.find((b) => b.index > index);
    const block = [...(name ? [`# ${name}`] : []), ...body.split('\n'), ''];
    const at = after ? after.start : lines.length;
    const before = lines.slice(0, at);
    while (!after && before.length && before[before.length - 1].trim() === '') before.pop();
    out = [...before, ...(after ? [] : ['']), ...block, ...lines.slice(at)];
  }
  let result = out.join('\n').replace(/\n{3,}/g, '\n\n');
  if (!result.endsWith('\n')) result += '\n';
  // mu-data deja una línea en blanco al final: si estaba, se conserva.
  if (/\n[ \t]*\n$/.test(String(text)) && !result.endsWith('\n\n')) result += '\n';
  return result;
}

function trimLeadingBlank(tail, hadReplacement) {
  if (!hadReplacement) return tail;
  let t = tail;
  while (t.length && t[0].trim() === '') t = t.slice(1);
  return t;
}

//**********************************************//
//******** Vista en filas (ItemValue/Stack) *****//
//**********************************************//

function itemKey(section, index) {
  return `${pad(section, 2)},${pad(index, 3)}`;
}

function levelToken(level) {
  return (level === undefined || level === '*') ? '*' : String(level);
}

function stackList(item) {
  if (item.stack === undefined) return [];
  return Array.isArray(item.stack) ? item.stack : [{ max: item.stack }];
}

// sections: { sección: { índice: item } } -> texto con el formato de
// ItemValue.txt / ItemStack.txt, para el editor de tabla del panel.
function buildRows(kind, sections, names = {}) {
  const lines = [kind === 'value' ? '//Index\t\tLevel\t\tBuyValue\tSellValue\tComment' : '//Index\t\tLevel\t\tMaxStack\tCreateIndex\tComment'];
  for (const section of Object.keys(sections).map(Number).sort((a, b) => a - b)) {
    for (const index of Object.keys(sections[section]).map(Number).sort((a, b) => a - b)) {
      const item = sections[section][index];
      const name = names[`${section},${index}`];
      const comment = name ? `\t\t// ${name}` : '';
      if (kind === 'value') {
        for (const v of item.value || []) {
          lines.push(`${itemKey(section, index)}\t\t${levelToken(v.level)}\t\t${v.buy ?? '*'}\t\t${v.sell ?? '*'}${comment}`);
        }
      } else {
        for (const s of stackList(item)) {
          const pair = s.create === undefined ? null : parseItemPair(s.create);
          lines.push(`${itemKey(section, index)}\t\t${levelToken(s.level)}\t\t${s.max}\t\t${pair ? itemKey(pair[0], pair[1]) : '*'}${comment}`);
        }
      }
    }
  }
  lines.push('end');
  return lines.join('\n');
}

function rowNumber(token, where) {
  if (token === '*') return undefined;
  if (!/^-?\d+$/.test(token)) throw new ItemExtrasError(`${where}: '${token}' no es un número`);
  const n = Number(token);
  return n < 0 ? undefined : n;
}

// Texto de la tabla -> { "S,I": [entradas] } (value: { level, buy, sell };
// stack: { level, max, create }).
function parseRows(kind, text) {
  const out = {};
  const lines = String(text || '').split(/\r?\n/);
  lines.forEach((raw, n) => {
    const where = `línea ${n + 1}`;
    const line = raw.split('//')[0].trim();
    if (!line) return;
    if (line.toLowerCase() === 'end') { lines.length = 0; return; }
    const tokens = line.split(/\s+/);
    if (tokens.length < 4) throw new ItemExtrasError(`${where}: se esperan 4 columnas`);
    const key = parseItemPair(tokens[0]);
    if (!key) throw new ItemExtrasError(`${where}: '${tokens[0]}' no es Sección,Índice`);
    const level = rowNumber(tokens[1], where);
    if (level !== undefined && level > 15) throw new ItemExtrasError(`${where}: el nivel va de 0 a 15 o *`);
    const entry = level === undefined ? {} : { level };
    if (kind === 'value') {
      const buy = rowNumber(tokens[2], where);
      const sell = rowNumber(tokens[3], where);
      if (buy !== undefined) entry.buy = buy;
      if (sell !== undefined) entry.sell = sell;
    } else {
      const max = rowNumber(tokens[2], where);
      if (max === undefined || max > 255) throw new ItemExtrasError(`${where}: MaxStack va de 0 a 255`);
      entry.max = max;
      if (tokens[3] !== '*') {
        const create = parseItemPair(tokens[3]);
        if (!create) throw new ItemExtrasError(`${where}: CreateIndex tiene que ser Sección,Índice o *`);
        entry.create = `${create[0]},${create[1]}`;
      }
    }
    const k = `${key[0]},${key[1]}`;
    (out[k] = out[k] || []).push(entry);
  });
  return out;
}

// Aplica la tabla a los archivos de sección. files: { sección: texto }.
// Devuelve { sección: texto nuevo } sólo de los que cambiaron.
function applyRows(kind, rows, files, names = {}) {
  const field = kind === 'value' ? 'value' : 'stack';
  const changed = {};
  const sections = new Set([...Object.keys(files).map(Number), ...Object.keys(rows).map((k) => Number(k.split(',')[0]))]);
  for (const section of sections) {
    let text = files[section] || '';
    const items = parseSection(text, sectionFileName(section));
    const touched = new Set([
      ...Object.keys(items).map(Number).filter((i) => items[i][field] !== undefined),
      ...Object.keys(rows).filter((k) => Number(k.split(',')[0]) === section).map((k) => Number(k.split(',')[1]))
    ]);
    for (const index of [...touched].sort((a, b) => a - b)) {
      const item = { ...(items[index] || {}) };
      const list = rows[`${section},${index}`];
      if (!list || !list.length) delete item[field];
      else if (field === 'stack' && list.length === 1 && list[0].level === undefined && list[0].create === undefined) item.stack = list[0].max;
      else item[field] = list;
      text = replaceItem(text, section, index, item, names[`${section},${index}`] || '');
    }
    if (text !== (files[section] || '')) changed[section] = text;
  }
  return changed;
}

module.exports = {
  ITEMS_DIR,
  SECTION_NAMES,
  ItemExtrasError,
  sectionFileName,
  sectionPath,
  fileSection,
  parseSection,
  parseItemPair,
  replaceItem,
  itemBody,
  buildRows,
  parseRows,
  applyRows,
};
