'use strict';

// Items custom: lectura, validación y escritura de los JSON de
// Data/Custom/Items/<sección>_<índice>.json (efectos, pose de inventario y tooltip)
// y Data/Custom/Pets/<sección>_<índice>.json (movimiento y habilidades).
//
// Las reglas de validación son las que aplica el server al arrancar
// (GameServer/ContentCatalog.cpp y CustomPet.cpp): un archivo que no las cumple se
// descarta entero, así que el panel no tiene que dejar guardar nada que el server
// vaya a rechazar.

const KINDS = { items: 'Data/Custom/Items', pets: 'Data/Custom/Pets' };

const MAX_SECTION = 15;
const MAX_INDEX = 511;
const MAX_EFFECT_BONES = 32;
const MAX_TOOLTIP_LINES = 6;
const MAX_TOOLTIP_BYTES = 59; // Text[60] con el 0 final
const TOOLTIP_COLORS = ['white', 'blue', 'red', 'gold', 'green', 'darkred', 'purple', 'darkblue', 'darkgold'];
const HELPER_SLOT = 8;

const pad = (n, width) => String(n).padStart(width, '0');

function isKind(kind) {
  return Object.hasOwn(KINDS, kind);
}

function validKey(key) {
  return Array.isArray(key) && key.length === 2
    && key.every((n) => Number.isInteger(n) && n >= 0)
    && key[0] <= MAX_SECTION && key[1] <= MAX_INDEX;
}

// "12,32" / "12,032" / [12, 32] -> [12, 32], o null.
function parseKey(value) {
  let section;
  let index;
  if (typeof value === 'string') {
    const match = value.trim().match(/^(\d+)\s*,\s*(\d+)$/);
    if (!match) return null;
    section = Number(match[1]);
    index = Number(match[2]);
  } else if (Array.isArray(value) && value.length === 2) {
    [section, index] = value;
  } else {
    return null;
  }
  return validKey([section, index]) ? [section, index] : null;
}

function fileName(key) {
  return `${pad(key[0], 2)}_${pad(key[1], 3)}.json`;
}

function pathFor(kind, key) {
  if (!isKind(kind) || !validKey(key)) throw new Error('Tipo o item inválido');
  return `${KINDS[kind]}/${fileName(key)}`;
}

// Nombre de archivo -> [sección, índice]; null si no tiene la forma SS_III.json.
function keyFromFileName(name) {
  const match = String(name).match(/^(\d{2})_(\d{3})\.json$/);
  if (!match) return null;
  const key = [Number(match[1]), Number(match[2])];
  return validKey(key) ? key : null;
}

// ── JSON con comentarios ─────────────────────────────────────────────────────

// Quita los comentarios // y /* */ que no estén dentro de una cadena.
function stripComments(text) {
  let out = '';
  let inString = false;
  for (let i = 0; i < text.length; i += 1) {
    const c = text[i];
    if (inString) {
      out += c;
      if (c === '\\') { out += text[i + 1] ?? ''; i += 1; } else if (c === '"') inString = false;
    } else if (c === '"') {
      inString = true;
      out += c;
    } else if (c === '/' && text[i + 1] === '/') {
      while (i < text.length && text[i] !== '\n') i += 1;
      out += '\n';
    } else if (c === '/' && text[i + 1] === '*') {
      i += 2;
      while (i < text.length && !(text[i] === '*' && text[i + 1] === '/')) i += 1;
      i += 1;
    } else {
      out += c;
    }
  }
  return out;
}

// { header: ['// ...', ...], data, hasInnerComments } o { error }. El encabezado son
// los comentarios del principio del archivo: se conservan al guardar. Los que estén
// adentro del JSON se pierden, y por eso se avisa con hasInnerComments.
function parseJsonFile(text) {
  const source = String(text ?? '').replace(/^\uFEFF/, '');
  const lines = source.split(/\r?\n/);
  const header = [];
  let consumed = 0;
  while (consumed < lines.length) {
    const trimmed = lines[consumed].trim();
    if (trimmed.startsWith('//')) header.push(trimmed);
    else if (trimmed !== '') break;
    consumed += 1;
  }
  const body = lines.slice(consumed).join('\n');
  try {
    const data = JSON.parse(stripComments(source));
    return { header, data, hasInnerComments: body !== stripComments(body) };
  } catch (err) {
    return { error: err.message };
  }
}

// ── Formato de salida ────────────────────────────────────────────────────────

const isScalar = (v) => v === null || typeof v !== 'object';
const isInlineArray = (v) => Array.isArray(v) && v.length <= 12 && v.every(isScalar);

function format(value, depth) {
  const indent = '  '.repeat(depth);
  const inner = '  '.repeat(depth + 1);
  if (isScalar(value)) return JSON.stringify(value);
  if (Array.isArray(value)) {
    if (value.length === 0) return '[]';
    if (isInlineArray(value)) return `[${value.map((v) => JSON.stringify(v)).join(', ')}]`;
    // Las líneas de tooltip, cortas, van cada una en su renglón y en una sola línea.
    return `[\n${value.map((v) => inner + format(v, depth + 1)).join(',\n')}\n${indent}]`;
  }
  const keys = Object.keys(value);
  if (keys.length === 0) return '{}';
  const oneLine = keys.every((k) => isScalar(value[k]) || isInlineArray(value[k]));
  const text = keys.map((k) => `"${k}": ${format(value[k], depth + 1)}`);
  if (oneLine && depth >= 1 && text.join(', ').length <= 90) return `{ ${text.join(', ')} }`;
  return `{\n${text.map((t) => inner + t).join(',\n')}\n${indent}}`;
}

function serializeJsonFile(header, data) {
  const lines = (header || []).filter((l) => l.startsWith('//'));
  return (lines.length ? `${lines.join('\r\n')}\r\n` : '') + format(data, 0).replace(/\n/g, '\r\n') + '\r\n';
}

// ── Validación ───────────────────────────────────────────────────────────────

const isNumber = (v) => typeof v === 'number' && Number.isFinite(v);

function vector(object, key, size, errors, where) {
  const value = object[key];
  if (value === undefined) return;
  if (!Array.isArray(value) || value.length !== size) { errors.push(`${where}: '${key}' tiene que ser un array de ${size} números`); return; }
  if (!value.every(isNumber)) errors.push(`${where}: '${key}' tiene que ser numérico`);
}

// Misma regla que IsSafeModelPath del server: letras, números y \ / _ - . espacio.
function safePath(text) {
  return typeof text === 'string' && text.length > 0 && !text.includes('..') && /^[A-Za-z0-9\\/_\-. ]+$/.test(text);
}

function validateEffect(effect, n, errors) {
  const where = `Efecto ${n + 1}`;
  if (effect === null || typeof effect !== 'object' || Array.isArray(effect)) { errors.push(`${where}: tiene que ser un objeto`); return; }
  if ((effect.on ?? 'equipped') !== 'equipped') errors.push(`${where}: 'on' solo admite 'equipped'`);
  if (effect.type !== 'sprite' && effect.type !== 'particle') errors.push(`${where}: 'type' tiene que ser 'sprite' o 'particle'`);
  if (effect.type === 'particle') {
    if (!isNumber(effect.particle) || effect.particle < 0 || effect.particle > 0xFFFF) errors.push(`${where}: una partícula necesita 'particle' (tipo vanilla, 0 a 65535)`);
    if (effect.subtype !== undefined && !isNumber(effect.subtype)) errors.push(`${where}: 'subtype' tiene que ser numérico`);
  }
  const hasTexture = typeof effect.texture === 'string' && effect.texture !== '';
  if (hasTexture) {
    if (effect.texture.length >= 48 || !safePath(effect.texture)) errors.push(`${where}: ruta de textura inválida`);
  } else if (!(isNumber(effect.bitmap) && effect.bitmap >= 0 && effect.bitmap < 0xFFFF) && effect.type === 'sprite') {
    errors.push(`${where}: un sprite necesita 'texture' o 'bitmap'`);
  }
  if (!Array.isArray(effect.bones) || effect.bones.length < 1 || effect.bones.length > MAX_EFFECT_BONES) {
    errors.push(`${where}: 'bones' tiene que ser un array de 1 a ${MAX_EFFECT_BONES} huesos`);
  } else if (!effect.bones.every((b) => isNumber(b) && Number.isInteger(b) && b >= 0 && b <= 199)) {
    errors.push(`${where}: hueso inválido en 'bones' (0 a 199)`);
  }
  if (effect.chance !== undefined && !isNumber(effect.chance)) errors.push(`${where}: 'chance' tiene que ser numérico`);
  if (effect.scale !== undefined && !isNumber(effect.scale)) errors.push(`${where}: 'scale' tiene que ser numérico`);
  vector(effect, 'color', 3, errors, where);
  vector(effect, 'offset', 3, errors, where);
  if (effect.pulse !== undefined) {
    if (effect.pulse === null || typeof effect.pulse !== 'object' || Array.isArray(effect.pulse)) errors.push(`${where}: 'pulse' tiene que ser un objeto`);
    else for (const k of ['speed', 'scale', 'color']) if (effect.pulse[k] !== undefined && !isNumber(effect.pulse[k])) errors.push(`${where}: 'pulse.${k}' tiene que ser numérico`);
  }
}

// Devuelve la lista de errores (vacía si el archivo es válido).
function validateItemJson(data) {
  const errors = [];
  if (data === null || typeof data !== 'object' || Array.isArray(data)) return ['El archivo tiene que ser un objeto'];
  if (!parseKey(data.item)) errors.push("Falta 'item' o es inválido (por ejemplo \"13,400\")");
  if (data.effects !== undefined) {
    if (!Array.isArray(data.effects)) errors.push("'effects' tiene que ser un array");
    else data.effects.forEach((e, n) => validateEffect(e, n, errors));
  }
  if (data.inventory !== undefined) {
    const inv = data.inventory;
    if (inv === null || typeof inv !== 'object' || Array.isArray(inv)) errors.push("'inventory' tiene que ser un objeto");
    else {
      vector(inv, 'position', 2, errors, 'Inventario');
      vector(inv, 'rotation', 3, errors, 'Inventario');
      if (inv.scale !== undefined && !isNumber(inv.scale)) errors.push("Inventario: 'scale' tiene que ser numérico");
    }
  }
  if (data.tooltip !== undefined) {
    if (!Array.isArray(data.tooltip) || data.tooltip.length > MAX_TOOLTIP_LINES) errors.push(`'tooltip' tiene que ser un array de hasta ${MAX_TOOLTIP_LINES} líneas`);
    else {
      data.tooltip.forEach((line, n) => {
        const text = typeof line === 'string' ? line : line?.text;
        const color = typeof line === 'string' ? 'white' : (line?.color ?? 'white');
        if (typeof text !== 'string' || text === '') errors.push(`Tooltip, línea ${n + 1}: falta el texto`);
        else if (Buffer.byteLength(text, 'utf8') > MAX_TOOLTIP_BYTES) errors.push(`Tooltip, línea ${n + 1}: el texto pasa de ${MAX_TOOLTIP_BYTES} bytes`);
        if (!TOOLTIP_COLORS.includes(color)) errors.push(`Tooltip, línea ${n + 1}: color desconocido (${TOOLTIP_COLORS.join(', ')})`);
      });
    }
  }
  return errors;
}

// `itemSlot`: columna Slot de Item.txt (el server exige 8 para un pet).
function validatePetJson(data, { itemSlot } = {}) {
  const errors = [];
  if (data === null || typeof data !== 'object' || Array.isArray(data)) return ['El archivo tiene que ser un objeto'];
  if (!parseKey(data.item)) errors.push("Falta 'item' o es inválido (por ejemplo \"13,400\")");
  else if (itemSlot !== undefined && itemSlot !== HELPER_SLOT) errors.push(`El item tiene que ir en el slot del helper (Slot ${HELPER_SLOT} en Item.txt) y tiene Slot ${itemSlot}`);
  for (const k of ['scale', 'action', 'blendMesh']) if (data[k] !== undefined && !isNumber(data[k])) errors.push(`'${k}' tiene que ser numérico`);
  const movement = data.movement;
  if (movement === null || typeof movement !== 'object' || Array.isArray(movement)) errors.push("Falta 'movement'");
  else {
    if (movement.type !== 'follow' && movement.type !== 'orbit') errors.push("'movement.type' tiene que ser 'follow' u 'orbit'");
    for (const k of ['radius', 'period', 'height']) if (movement[k] !== undefined && !isNumber(movement[k])) errors.push(`'movement.${k}' tiene que ser numérico`);
    if (isNumber(movement.period ?? 4000) && (movement.period ?? 4000) < 100) errors.push("'movement.period' tiene que ser de al menos 100 ms");
  }
  if (data.abilities !== undefined) {
    if (!Array.isArray(data.abilities)) errors.push("'abilities' tiene que ser un array");
    else {
      data.abilities.forEach((ability, n) => {
        const where = `Habilidad ${n + 1}`;
        if (ability === null || typeof ability !== 'object') { errors.push(`${where}: tiene que ser un objeto`); return; }
        if (ability.type !== 'pickup') { errors.push(`${where}: habilidad desconocida (solo 'pickup')`); return; }
        if ((ability.what ?? 'zen') !== 'zen') errors.push(`${where}: 'pickup' por ahora solo levanta 'zen'`);
        const range = ability.range ?? 3;
        if (!isNumber(range) || range < 1 || range > 8) errors.push(`${where}: 'range' va de 1 a 8 casillas`);
        for (const k of ['interval', 'delay']) if (ability[k] !== undefined && !isNumber(ability[k])) errors.push(`${where}: '${k}' tiene que ser numérico`);
      });
    }
  }
  return errors;
}

// ── Lista de items (Data/Item/Item.txt) ──────────────────────────────────────

// [{ key: [sección, índice], name, slot }]. Las filas son:
//   Índice Slot Skill Ancho Alto Serial Opción Drop "Nombre" ...
function parseItemList(content) {
  const out = [];
  let section = -1;
  for (const raw of String(content ?? '').replace(/^﻿/, '').split(/\r?\n/)) {
    const line = raw.trim();
    if (line === '' || line.startsWith('//')) continue;
    if (line.toLowerCase() === 'end') { section = -1; continue; }
    if (/^\d+$/.test(line)) { section = Number(line); continue; }
    const match = line.match(/^(\d+)\s+(-?\d+)\s+(-?\d+)\s+\d+\s+\d+\s+\d+\s+\d+\s+\d+\s+"(.*?)"/);
    if (match && section >= 0) out.push({ key: [section, Number(match[1])], name: match[4], slot: Number(match[2]) });
  }
  return out;
}

module.exports = {
  KINDS,
  TOOLTIP_COLORS,
  MAX_TOOLTIP_LINES,
  isKind,
  validKey,
  parseKey,
  fileName,
  pathFor,
  keyFromFileName,
  parseJsonFile,
  serializeJsonFile,
  validateItemJson,
  validatePetJson,
  parseItemList
};
