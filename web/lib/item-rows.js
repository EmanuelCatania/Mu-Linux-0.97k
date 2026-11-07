'use strict';

// Filas de Data/Item/Item.txt: leer, validar, agregar y reemplazar sin tocar el
// resto del archivo.
//
// El formato es el que lee GameServer/ItemManager.cpp:
//
//   Índice Slot Skill Ancho Alto Serial Opción Drop "Nombre"  <columnas de la sección>
//   [Comportamiento] ["Carpeta"] ["Modelo"] [Brillo R G B] ["Carpeta puesto"] ["Modelo puesto"] [Gate]
//
// Las columnas de la sección dependen del número de sección (armas, escudos, alas,
// pociones...). Las del final son opcionales y cada una admite '*' (= la original).
// Los separadores son espacio, tabulación o coma ("00,020" son dos columnas), y un
// texto entre comillas llega hasta la próxima comilla.

const MAX_SECTION = 15;
const MAX_INDEX = 511;
const MAX_NAME_BYTES = 31; // ITEM_INFO::Name[32]
const CLASSES = ['DW', 'DK', 'ELF', 'MG'];
const PREFIX = ['Slot', 'Skill', 'Width', 'Height', 'Serial', 'Option', 'Drop'];
const REQUIRE = ['ReqLevel', 'ReqStr', 'ReqDex', 'ReqEne', 'ReqVit'];

// Columnas numéricas que siguen al nombre, por sección (en el orden del archivo).
function statColumns(section) {
  if (!Number.isInteger(section) || section < 0 || section > MAX_SECTION) throw new Error('Sección inválida');
  if (section <= 5) {
    return ['Level', 'DmgMin', 'DmgMax', 'AtkSpeed', 'Durability', 'MagicDurability', 'MagicDmgRate', ...REQUIRE, 'Atrib', ...CLASSES];
  }
  if (section === 6) return ['Level', 'Defense', 'DefenseRate', 'Durability', ...REQUIRE, 'Atrib', ...CLASSES];
  if (section <= 9) return ['Level', 'Defense', 'MagicDefense', 'Durability', ...REQUIRE, 'Atrib', ...CLASSES];
  if (section === 10) return ['Level', 'Defense', 'AtkSpeed', 'Durability', ...REQUIRE, 'Atrib', ...CLASSES];
  if (section === 11) return ['Level', 'Defense', 'WalkSpeed', 'Durability', ...REQUIRE, 'Atrib', ...CLASSES];
  if (section === 12) return ['Level', 'Defense', 'Durability', 'ReqLevel', 'ReqEne', 'ReqStr', 'ReqDex', 'BuyMoney', ...CLASSES];
  if (section === 13) return ['Level', 'Durability', 'ResIce', 'ResPoison', 'ResLightning', 'ResFire', 'Atrib', ...CLASSES];
  if (section === 14) return ['Value', 'Level'];
  return ['Level', 'ReqLevel', 'ReqEne', 'BuyMoney', ...CLASSES];
}

// Cantidad de columnas antes de las opcionales: índice + prefijo + nombre + las de la sección.
function baseCount(section) {
  return 1 + PREFIX.length + 1 + statColumns(section).length;
}

// ── Tokens (misma regla que CReadScript::SplitColumns) ───────────────────────

// Parte una fila en columnas. Devuelve { tokens: [{ text, quoted }], comment }.
function tokenize(line) {
  const tokens = [];
  let comment = '';
  let i = 0;
  const n = line.length;
  while (i < n) {
    while (i < n && (line[i] === ' ' || line[i] === '\t' || line[i] === ',')) i += 1;
    if (i >= n) break;
    if (line[i] === '/' && line[i + 1] === '/') { comment = line.slice(i); break; }
    if (line[i] === '"') {
      i += 1;
      const start = i;
      while (i < n && line[i] !== '"') i += 1;
      tokens.push({ text: line.slice(start, i), quoted: true });
      i += 1;
    } else {
      const start = i;
      while (i < n && line[i] !== ' ' && line[i] !== '\t' && line[i] !== ',') i += 1;
      tokens.push({ text: line.slice(start, i), quoted: false });
    }
  }
  return { tokens, comment };
}

const isStar = (token) => token !== undefined && !token.quoted && token.text === '*';
const pad = (value, width) => String(value).padStart(width, '0');

// Misma regla que IsSafeModelPath del server.
function safePath(text) {
  return typeof text === 'string' && text.length > 0 && !text.includes('..') && /^[A-Za-z0-9\\/_\-. ]+$/.test(text);
}

// ── Columnas opcionales ──────────────────────────────────────────────────────

function parseTail(tail) {
  const out = { behavior: null, folder: null, model: null, glow: null, equippedFolder: null, equippedModel: null, gate: null };
  let n = 0;
  const count = tail.length;
  if (n < count) {
    if (isStar(tail[n])) n += 1;
    else if (n + 1 < count) { out.behavior = [parseInt(tail[n].text, 10) || 0, parseInt(tail[n + 1].text, 10) || 0]; n += 2; }
  }
  const text = () => {
    const token = tail[n];
    n += 1;
    return token === undefined || isStar(token) ? null : token.text;
  };
  if (n < count) out.folder = text();
  if (n < count) out.model = text();
  if (n < count) {
    if (isStar(tail[n])) n += 1;
    else if (n + 2 < count) { out.glow = [0, 1, 2].map((k) => parseInt(tail[n + k].text, 10) || 0); n += 3; }
  }
  if (n < count) out.equippedFolder = text();
  if (n < count) out.equippedModel = text();
  if (n < count) {
    const token = tail[n];
    out.gate = isStar(token) ? null : (parseInt(token.text, 10) || 0);
  }
  return out;
}

function tailTokens(tail) {
  const q = (s) => `"${s}"`;
  const tokens = [
    tail.behavior ? `${pad(tail.behavior[0], 2)},${pad(tail.behavior[1], 3)}` : '*',
    tail.folder ? q(tail.folder) : '*',
    tail.model ? q(tail.model) : '*',
    ...(tail.glow ? tail.glow.map(String) : ['*']),
    tail.equippedFolder ? q(tail.equippedFolder) : '*',
    tail.equippedModel ? q(tail.equippedModel) : '*',
    tail.gate !== null && tail.gate !== undefined ? String(tail.gate) : '*'
  ];
  while (tokens.length && tokens[tokens.length - 1] === '*') tokens.pop();
  return tokens;
}

// ── Archivo completo ─────────────────────────────────────────────────────────

// Estructura del archivo: { lines, eol, sections: { [n]: { header, end, rows: [...] } } }
// Cada fila trae su línea, para poder reemplazarla o insertar a su lado.
function parseItemFile(content) {
  const text = String(content ?? '').replace(/^﻿/, '');
  const eol = /\r\n/.test(text) ? '\r\n' : '\n';
  const lines = text.split(/\r?\n/);
  if (lines.length && lines[lines.length - 1] === '') lines.pop();
  const sections = {};
  let current = null;
  lines.forEach((raw, lineIndex) => {
    const line = raw.trim();
    if (line === '' || line.startsWith('//')) return;
    if (/^\d+$/.test(line)) {
      current = { number: Number(line), header: lineIndex, end: -1, rows: [] };
      sections[current.number] = current;
      return;
    }
    if (line.toLowerCase() === 'end') {
      if (current) current.end = lineIndex;
      current = null;
      return;
    }
    if (!current) return;
    const { tokens, comment } = tokenize(raw);
    const need = baseCount(current.number);
    if (tokens.length < need || !/^\d+$/.test(tokens[0].text)) {
      current.rows.push({ lineIndex, malformed: true, line: raw });
      return;
    }
    const stats = tokens.slice(1 + PREFIX.length + 1, need).map((token) => token.text);
    current.rows.push({
      lineIndex,
      line: raw,
      comment,
      index: Number(tokens[0].text),
      slot: Number(tokens[1].text),
      skill: Number(tokens[2].text),
      width: Number(tokens[3].text),
      height: Number(tokens[4].text),
      serial: Number(tokens[5].text),
      option: Number(tokens[6].text),
      drop: Number(tokens[7].text),
      name: tokens[8].text,
      stats,
      tail: parseTail(tokens.slice(need))
    });
  });
  return { lines, eol, sections };
}

function findRow(file, key) {
  const section = file.sections[key[0]];
  return section ? section.rows.find((row) => !row.malformed && row.index === key[1]) : undefined;
}

// Modelo de una fila para el formulario.
function rowModel(section, row) {
  return {
    key: [section, row.index],
    name: row.name,
    slot: row.slot,
    skill: row.skill,
    width: row.width,
    height: row.height,
    serial: row.serial,
    option: row.option,
    drop: row.drop,
    stats: row.stats.map(Number),
    tail: row.tail
  };
}

const isInt = (v) => typeof v === 'number' && Number.isInteger(v) && Math.abs(v) <= 2147483647;

// Lista de errores del modelo (vacía si se puede escribir).
function validateRow(model) {
  const errors = [];
  if (!model || typeof model !== 'object') return ['Falta la fila'];
  const key = model.key;
  if (!Array.isArray(key) || key.length !== 2 || !isInt(key[0]) || !isInt(key[1]) || key[0] < 0 || key[0] > MAX_SECTION || key[1] < 0 || key[1] > MAX_INDEX) {
    return [`El item tiene que tener sección de 0 a ${MAX_SECTION} e índice de 0 a ${MAX_INDEX}`];
  }
  const columns = statColumns(key[0]);
  if (typeof model.name !== 'string' || model.name.trim() === '') errors.push('Falta el nombre');
  else {
    if (/["\r\n\t]|\/\//.test(model.name)) errors.push('El nombre no puede llevar comillas, tabulaciones, saltos de línea ni //');
    if (/[\p{Cc}\p{Cs}\p{Co}\p{Cn}\p{Zl}\p{Zp}]/u.test(model.name)) errors.push('El nombre lleva caracteres de control');
    if (Buffer.byteLength(model.name, 'utf8') > MAX_NAME_BYTES) errors.push(`El nombre pasa de ${MAX_NAME_BYTES} bytes`);
  }
  for (const field of ['slot', 'skill', 'width', 'height', 'serial', 'option', 'drop']) {
    if (!isInt(model[field])) errors.push(`${field}: tiene que ser un número entero`);
  }
  if (isInt(model.width) && (model.width < 1 || model.width > 8)) errors.push('Ancho: va de 1 a 8 casillas');
  if (isInt(model.height) && (model.height < 1 || model.height > 8)) errors.push('Alto: va de 1 a 8 casillas');
  if (!Array.isArray(model.stats) || model.stats.length !== columns.length) {
    errors.push(`La sección ${key[0]} tiene ${columns.length} columnas de datos (${columns.join(', ')})`);
  } else {
    model.stats.forEach((value, n) => { if (!isInt(value)) errors.push(`${columns[n]}: tiene que ser un número entero`); });
  }
  const tail = model.tail || {};
  if (tail.behavior) {
    if (!Array.isArray(tail.behavior) || tail.behavior.length !== 2 || !isInt(tail.behavior[0]) || !isInt(tail.behavior[1]) || tail.behavior[0] < 0 || tail.behavior[0] > MAX_SECTION || tail.behavior[1] < 0 || tail.behavior[1] > MAX_INDEX) {
      errors.push('Comportamiento: tiene que ser un item (sección e índice)');
    }
  } else if (key[1] >= 32) {
    errors.push('Un item agregado (índice 32 o más) tiene que decir qué item original imita (Comportamiento)');
  }
  for (const field of ['folder', 'model', 'equippedFolder', 'equippedModel']) {
    if (tail[field] !== null && tail[field] !== undefined && tail[field] !== '' && !safePath(tail[field])) errors.push(`${field}: ruta inválida (letras, números y \\ / _ - . espacio)`);
  }
  if (tail.glow) {
    if (!Array.isArray(tail.glow) || tail.glow.length !== 3 || !tail.glow.every((c) => isInt(c) && c >= 0 && c <= 255)) errors.push('Brillo: tres números de 0 a 255');
  }
  if (tail.gate !== null && tail.gate !== undefined && (!isInt(tail.gate) || tail.gate < 0)) errors.push('Gate: tiene que ser un número entero positivo');
  // Si hay carpeta, tiene que haber modelo (y al revés): el cliente necesita los dos.
  if (!!tail.folder !== !!tail.model) errors.push('La carpeta y el modelo van juntos');
  if (!!tail.equippedFolder !== !!tail.equippedModel) errors.push('La carpeta y el modelo puesto van juntos');
  return errors;
}

// Línea de la fila, separada por tabulaciones como las originales.
function buildRowLine(model, comment) {
  const q = (s) => `"${s}"`;
  const parts = [
    String(model.key[1]), String(model.slot), String(model.skill), String(model.width), String(model.height),
    String(model.serial), String(model.option), String(model.drop), q(model.name),
    ...model.stats.map(String),
    ...tailTokens(model.tail || {})
  ];
  return parts.join('\t') + (comment ? `\t${comment}` : '');
}

// Primer índice libre (a partir de `from`) de la sección.
function nextFreeIndex(file, section, from = 32) {
  const used = new Set((file.sections[section]?.rows || []).filter((r) => !r.malformed).map((r) => r.index));
  for (let i = from; i <= MAX_INDEX; i += 1) if (!used.has(i)) return i;
  return -1;
}

// Agrega una fila nueva a su sección, en orden de índice. Devuelve el contenido nuevo.
function addRow(content, model) {
  const errors = validateRow(model);
  if (errors.length) return { errors };
  const file = parseItemFile(content);
  const section = file.sections[model.key[0]];
  if (!section || section.end < 0) return { errors: [`No se encuentra la sección ${model.key[0]} en Item.txt`] };
  if (findRow(file, model.key)) return { errors: [`El item ${model.key[0]},${model.key[1]} ya existe`] };
  const prior = section.rows.filter((r) => !r.malformed && r.index < model.key[1]);
  const at = prior.length ? prior[prior.length - 1].lineIndex + 1 : section.rows.length ? section.rows[0].lineIndex : section.end;
  const lines = file.lines.slice();
  lines.splice(at, 0, buildRowLine(model));
  return { content: lines.join(file.eol) + file.eol };
}

// Reemplaza una fila existente (conserva su comentario de al final).
function replaceRow(content, model) {
  const errors = validateRow(model);
  if (errors.length) return { errors };
  const file = parseItemFile(content);
  const row = findRow(file, model.key);
  if (!row) return { errors: [`El item ${model.key[0]},${model.key[1]} no existe`] };
  const lines = file.lines.slice();
  lines[row.lineIndex] = buildRowLine(model, row.comment);
  return { content: lines.join(file.eol) + file.eol };
}

module.exports = {
  MAX_NAME_BYTES,
  CLASSES,
  PREFIX,
  statColumns,
  baseCount,
  tokenize,
  parseTail,
  tailTokens,
  parseItemFile,
  findRow,
  rowModel,
  validateRow,
  buildRowLine,
  nextFreeIndex,
  addRow,
  replaceRow
};
