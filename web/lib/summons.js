// Invocaciones de la elfa (Orbe de Invocación +0..+10) en
// GameServer/Data/GameServerInfo - Skill.dat:
//
//   SummonMonster1..11   monstruo (índice de Monster.txt) de cada nivel del orbe
//   SummonConstA/B/C     escalado: % = A + Energía / B + Nivel / C
//   SummonResetRate      % por cada reset (el nivel vuelve a 1 al resetear)
//   SummonMaxRate        tope del porcentaje
//
// El panel lee y escribe sólo esas claves: el resto del archivo (comentarios,
// otras skills, finales de línea) queda como estaba. Los valores por defecto
// son los mismos que usa el GameServer cuando falta la clave (ServerInfo.cpp).

const SLOT_COUNT = 11;
const SLOT_DEFAULTS = [26, 32, 21, 20, 10, 150, 151, 52, 48, 58, 61];
const CONST_KEYS = ['SummonConstA', 'SummonConstB', 'SummonConstC', 'SummonResetRate', 'SummonMaxRate'];
const CONST_DEFAULTS = { SummonConstA: 0, SummonConstB: 20, SummonConstC: 2, SummonResetRate: 0, SummonMaxRate: 300 };

class SummonError extends Error {}

function slotKey(n) {
  return `SummonMonster${n + 1}`;
}

function readKeys(content) {
  const values = new Map();
  for (const raw of String(content || '').split(/\r?\n/)) {
    const line = raw.replace(/;.*$/, '').trim();
    const m = line.match(/^([A-Za-z0-9_]+)\s*=\s*(-?\d+)/);
    if (m) values.set(m[1].toLowerCase(), Number(m[2]));
  }
  return values;
}

// Estado del panel a partir del texto del .dat.
function parse(content) {
  const values = readKeys(content);
  const pick = (key, fallback) => (values.has(key.toLowerCase()) ? values.get(key.toLowerCase()) : fallback);
  const slots = [];
  for (let n = 0; n < SLOT_COUNT; n++) slots.push(pick(slotKey(n), SLOT_DEFAULTS[n]));
  const constants = {};
  for (const key of CONST_KEYS) constants[key] = pick(key, CONST_DEFAULTS[key]);
  return { slots, constants };
}

function checkInt(value, label, min, max) {
  const n = Number(value);
  if (!Number.isInteger(n) || n < min || n > max) {
    throw new SummonError(`${label}: tiene que ser un entero entre ${min} y ${max}`);
  }
  return n;
}

// Valida lo que manda el panel. `monsterIds` es el conjunto de índices de
// Monster.txt; vacío = no se valida contra la lista.
function validate(input, monsterIds) {
  const slots = Array.isArray(input?.slots) ? input.slots : [];
  if (slots.length !== SLOT_COUNT) throw new SummonError(`Hacen falta ${SLOT_COUNT} invocaciones`);
  const out = { slots: [], constants: {} };
  slots.forEach((value, n) => {
    const id = checkInt(value, `Orbe +${n}`, 0, 1023);
    if (id > 0 && monsterIds && monsterIds.size && !monsterIds.has(id)) {
      throw new SummonError(`Orbe +${n}: el monstruo ${id} no está en Monster.txt`);
    }
    out.slots.push(id);
  });
  out.constants.SummonConstA = checkInt(input?.constants?.SummonConstA, 'SummonConstA', 0, 1000);
  out.constants.SummonConstB = checkInt(input?.constants?.SummonConstB, 'SummonConstB', 0, 100000);
  out.constants.SummonConstC = checkInt(input?.constants?.SummonConstC, 'SummonConstC', 0, 100000);
  out.constants.SummonResetRate = checkInt(input?.constants?.SummonResetRate, 'SummonResetRate', 0, 10000);
  out.constants.SummonMaxRate = checkInt(input?.constants?.SummonMaxRate, 'SummonMaxRate', 0, 10000);
  return out;
}

// Escribe los valores en el texto del .dat: reemplaza la clave donde está y,
// si falta, la agrega después de la última clave Summon* (o al final).
function apply(content, state) {
  const eol = /\r\n/.test(content) ? '\r\n' : '\n';
  const lines = String(content || '').split(/\r?\n/);
  const wanted = new Map();
  state.slots.forEach((v, n) => wanted.set(slotKey(n), v));
  for (const key of CONST_KEYS) wanted.set(key, state.constants[key]);

  const done = new Set();
  let lastSummon = -1;
  for (let i = 0; i < lines.length; i++) {
    const m = lines[i].match(/^(\s*)([A-Za-z0-9_]+)(\s*=\s*)(-?\d+)(.*)$/);
    if (!m) continue;
    const key = [...wanted.keys()].find((k) => k.toLowerCase() === m[2].toLowerCase());
    if (/^summon/i.test(m[2])) lastSummon = i;
    if (!key) continue;
    lines[i] = `${m[1]}${m[2]}${m[3]}${wanted.get(key)}${m[5]}`;
    done.add(key);
  }
  const missing = [...wanted.keys()].filter((k) => !done.has(k)).map((k) => `${k}=${wanted.get(k)}`);
  if (missing.length) {
    let at = lastSummon >= 0 ? lastSummon + 1 : lines.length;
    if (at === lines.length && lines.length && lines[lines.length - 1] === '') at -= 1;
    lines.splice(at, 0, ...missing);
  }
  return lines.join(eol);
}

// Porcentaje que gana una invocación (igual que CSkillManager::SkillSummon).
function rate(constants, energy, level, resets = 0) {
  let value = Number(constants.SummonConstA) || 0;
  if (constants.SummonConstB > 0) value += Math.floor(energy / constants.SummonConstB);
  if (constants.SummonConstC > 0) value += Math.floor(level / constants.SummonConstC);
  if (constants.SummonResetRate > 0 && resets > 0) value += resets * constants.SummonResetRate;
  return Math.min(value, Number(constants.SummonMaxRate) || 0);
}

// Monstruos de Monster.txt con su nivel, para el selector del panel.
function parseMonsters(content) {
  const out = [];
  for (const raw of String(content || '').split(/\r?\n/)) {
    const line = raw.trim();
    if (!line || line.startsWith('//')) continue;
    const tokens = line.match(/"[^"]*"|\S+/g);
    if (!tokens || tokens.length < 4) continue;
    const id = Number(tokens[0]);
    const at = tokens.findIndex((t) => t.startsWith('"'));
    if (!Number.isInteger(id) || at < 0) continue;
    out.push({ id, name: tokens[at].slice(1, -1), level: Number(tokens[at + 1]) || 0 });
  }
  return out;
}

module.exports = { SLOT_COUNT, SLOT_DEFAULTS, CONST_KEYS, CONST_DEFAULTS, SummonError, parse, validate, apply, rate, parseMonsters };
