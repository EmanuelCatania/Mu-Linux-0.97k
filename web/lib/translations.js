'use strict';

// Textos traducidos del server (Data/Text/<idioma>/*.txt): lectura, validación y
// escritura para el editor del panel.
//
// Las reglas son las mismas que aplica el server (Common/TextName.cpp) y el
// cliente (ClientText::SetServerText), para que el panel no deje guardar nada que
// después se descarte en silencio:
//   - UTF-8 en NFC, sin caracteres de control ni saltos de línea.
//   - Hasta 63 bytes (no caracteres: el campo del paquete F3:F0 es de 64 con el 0).
//   - En Client.txt, los %d/%s/%u/%.0f tienen que ser los del inglés, en el mismo
//     orden, porque el cliente los usa como formato de printf.

const LANGS = ['Eng', 'Spn', 'Por'];

const TABLES = {
  items: { file: 'Item.txt', keyFields: 2 },
  monsters: { file: 'Monster.txt', keyFields: 1 },
  skills: { file: 'Skill.txt', keyFields: 1 },
  client: { file: 'Client.txt', keyFields: 1 },
  // Líneas del tooltip de los items custom: sección, índice y número de línea. Entran
  // en el Text[60] de la línea del JSON (59 bytes).
  tooltips: { file: 'Tooltip.txt', keyFields: 3, maxBytes: 59 }
};

const MAX_TOOLTIP_LINE = 5;

const MAX_BYTES = 63;
const MAX_ITEM_SECTION = 15;
const MAX_ITEM_INDEX = 511;
const MAX_ID = 65535;

function isLang(lang) {
  return LANGS.includes(lang);
}

function isTable(table) {
  return Object.hasOwn(TABLES, table);
}

function pathFor(lang, table) {
  if (!isLang(lang) || !isTable(table)) throw new Error('Idioma o tabla inválidos');
  return `Data/Text/${lang}/${TABLES[table].file}`;
}

// Los archivos del server van en CRLF; uno vacío o que todavía no existe se crea igual.
function detectEol(content) {
  return /\n/.test(content) && !/\r\n/.test(content) ? '\n' : '\r\n';
}

// Encabezado de un archivo que se crea desde el editor.
function defaultHeader(table, lang) {
  const columns = table === 'tooltips' ? 'Sección\tÍndice\tLínea\tTexto'
    : TABLES[table].keyFields === 2 ? 'Sección\tÍndice\tNombre' : (table === 'client' ? 'Índice\tTexto' : 'Índice\tNombre');
  return [
    `//Textos traducidos (${lang}). Se mandan a los clientes con ese idioma; lo que no esté acá`,
    `//sale de los datos del cliente. UTF-8 sin BOM, hasta ${maxBytesFor(table)} bytes por texto.`,
    `//${columns}`
  ];
}

function keyString(key) {
  return key.join('\t');
}

function compareKeys(a, b) {
  for (let i = 0; i < Math.max(a.length, b.length); i += 1) {
    const diff = (a[i] ?? 0) - (b[i] ?? 0);
    if (diff !== 0) return diff;
  }
  return 0;
}

function validKey(table, key) {
  if (!Array.isArray(key) || key.length !== TABLES[table].keyFields) return false;
  if (!key.every((n) => Number.isInteger(n) && n >= 0)) return false;
  if (table === 'items') return key[0] <= MAX_ITEM_SECTION && key[1] <= MAX_ITEM_INDEX;
  if (table === 'tooltips') return key[0] <= MAX_ITEM_SECTION && key[1] <= MAX_ITEM_INDEX && key[2] <= MAX_TOOLTIP_LINE;
  return key[0] <= MAX_ID;
}

// Máximo de bytes de un texto de esa tabla.
function maxBytesFor(table) {
  return TABLES[table].maxBytes ?? MAX_BYTES;
}

// Lee el contenido de un archivo de traducciones. Lo que no se entiende queda en
// `errors` con su número de línea; el editor lo muestra y no lo reescribe.
function parseTranslations(content, table) {
  if (!isTable(table)) throw new Error('Tabla inválida');
  const text = String(content ?? '').replace(/^﻿/, '');
  const eol = detectEol(text);
  const fields = TABLES[table].keyFields;
  const pattern = new RegExp(`^${Array(fields).fill('(\\d+)').join('\\s+')}\\s+"(.*)"\\s*$`);

  const header = [];
  const entries = [];
  const errors = [];
  let pending = [];

  const lines = text.split(/\r?\n/);
  for (let i = 0; i < lines.length; i += 1) {
    const line = lines[i].trim();
    if (line === '') continue;
    if (line.toLowerCase() === 'end') break;
    if (line.startsWith('//')) {
      pending.push(lines[i].trimEnd());
      continue;
    }
    const match = line.match(pattern);
    if (!match) {
      errors.push({ line: i + 1, text: line.slice(0, 80) });
      continue;
    }
    const key = match.slice(1, 1 + fields).map(Number);
    const name = match[match.length - 1];
    if (entries.length === 0) {
      header.push(...pending);
      pending = [];
    }
    entries.push({ key, name, comments: pending });
    pending = [];
  }
  if (entries.length === 0) header.push(...pending);
  return { header, entries, errors, eol };
}

// Nombres de referencia (los que el server usa hoy) para mostrar junto a cada
// traducción. Sale de Item.txt, Monster.txt y Skill.txt del server, y de
// Eng/Client.txt para los textos del cliente.
function parseReference(content, table) {
  const text = String(content ?? '').replace(/^﻿/, '');
  const out = [];
  let section = -1;
  let label = '';
  for (const raw of text.split(/\r?\n/)) {
    const line = raw.trim();
    if (line === '') continue;
    if (line.startsWith('//')) {
      if (table === 'client') {
        const m = line.match(/^\/\/([A-Za-z][A-Za-z0-9]*)$/);
        if (m) label = m[1];
      }
      continue;
    }
    if (line.toLowerCase() === 'end') {
      if (table === 'items') section = -1;
      else break;
      continue;
    }
    if (table === 'items') {
      if (/^\d+$/.test(line)) { section = Number(line); continue; }
      const m = line.match(/^(\d+)\s+.*?"(.*?)"/);
      if (m && section >= 0) out.push({ key: [section, Number(m[1])], name: m[2] });
    } else if (table === 'monsters') {
      const m = line.match(/^(\d+)\s+\d+\s+"(.*?)"/);
      if (m) out.push({ key: [Number(m[1])], name: m[2] });
    } else if (table === 'skills') {
      const m = line.match(/^(\d+)\s+"(.*?)"/);
      if (m) out.push({ key: [Number(m[1])], name: m[2] });
    } else {
      const m = line.match(/^(\d+)\s+"(.*)"\s*$/);
      if (m) { out.push({ key: [Number(m[1])], name: m[2], label }); label = ''; }
    }
  }
  return out;
}

function formatSpecifiers(text) {
  const specs = [];
  const flags = '-+ #0123456789.*hlLzjt';
  for (let i = 0; i < text.length; i += 1) {
    if (text[i] !== '%') continue;
    if (text[i + 1] === '%') { i += 1; continue; }
    let j = i + 1;
    while (j < text.length && flags.includes(text[j])) j += 1;
    if (j < text.length) j += 1;
    specs.push(text.slice(i, j));
    i = j - 1;
  }
  return specs;
}

function sameFormatSpecifiers(a, b) {
  const sa = formatSpecifiers(a);
  const sb = formatSpecifiers(b);
  return sa.length === sb.length && sa.every((s, i) => s === sb[i]);
}

// Devuelve { name } con el texto ya normalizado, o { error } con el motivo.
function validateName(name, { reference, maxBytes = MAX_BYTES } = {}) {
  if (typeof name !== 'string') return { error: 'El texto no es una cadena' };
  const normalized = name.normalize('NFC');
  if (normalized === '') return { error: 'El texto está vacío' };
  if (/["\r\n\t]/.test(normalized)) return { error: 'No puede llevar comillas, tabulaciones ni saltos de línea' };
  if (/[\p{Cc}\p{Cs}\p{Co}\p{Cn}\p{Zl}\p{Zp}]/u.test(normalized)) return { error: 'Lleva caracteres de control o inválidos' };
  const bytes = Buffer.byteLength(normalized, 'utf8');
  if (bytes > maxBytes) return { error: `Mide ${bytes} bytes y el máximo es ${maxBytes}` };
  if (reference != null && !sameFormatSpecifiers(normalized, reference)) {
    return { error: 'Los %d/%s del texto no coinciden con los del inglés' };
  }
  return { name: normalized };
}

// Aplica los cambios del editor sobre lo que ya hay en el archivo.
//   edits: [{ key: [..], name }]   name vacío = quitar la traducción.
// Devuelve { entries } o { errors: [{ key, error }] }; no escribe nada si alguno falla.
function applyEdits(parsed, edits, table, { referenceFor } = {}) {
  const errors = [];
  const byKey = new Map(parsed.entries.map((e) => [keyString(e.key), e]));
  for (const edit of edits) {
    if (!edit || !validKey(table, edit.key)) {
      errors.push({ key: edit?.key ?? null, error: 'Clave inválida' });
      continue;
    }
    const id = keyString(edit.key);
    const name = typeof edit.name === 'string' ? edit.name.trim() : edit.name;
    if (name === '') {
      byKey.delete(id);
      continue;
    }
    const reference = referenceFor ? referenceFor(edit.key) : undefined;
    const checked = validateName(name, { reference, maxBytes: maxBytesFor(table) });
    if (checked.error) {
      errors.push({ key: edit.key, error: checked.error });
      continue;
    }
    const current = byKey.get(id);
    byKey.set(id, { key: edit.key, name: checked.name, comments: current ? current.comments : [] });
  }
  if (errors.length > 0) return { errors };
  const entries = [...byKey.values()].sort((a, b) => compareKeys(a.key, b.key));
  return { entries };
}

function serializeTranslations(parsed, entries, { table, lang } = {}) {
  const eol = parsed.eol || '\r\n';
  const lines = parsed.header.length === 0 && table ? defaultHeader(table, lang) : [...parsed.header];
  for (const entry of entries) {
    lines.push(...(entry.comments || []));
    lines.push(`${entry.key.join('\t')}\t"${entry.name}"`);
  }
  lines.push('end');
  return lines.join(eol) + eol;
}

module.exports = {
  LANGS,
  TABLES,
  MAX_BYTES,
  maxBytesFor,
  isLang,
  isTable,
  pathFor,
  parseTranslations,
  parseReference,
  validateName,
  sameFormatSpecifiers,
  applyEdits,
  serializeTranslations,
  defaultHeader
};
