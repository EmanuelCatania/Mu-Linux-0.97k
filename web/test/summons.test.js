const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const summons = require('../lib/summons');

const SKILL_DAT = path.join(__dirname, '..', '..', 'server', 'MuServer', 'GameServer', 'DATA', 'GameServerInfo - Skill.dat');

test('summons: el Skill.dat del repo trae las 11 invocaciones y el escalado', () => {
  const state = summons.parse(fs.readFileSync(SKILL_DAT, 'latin1'));
  assert.deepEqual(state.slots, summons.SLOT_DEFAULTS);
  assert.deepEqual(state.constants, summons.CONST_DEFAULTS);
});

test('summons: apply cambia sólo esas claves y conserva el resto', () => {
  const original = '; Skill\r\nHealConstA=5\r\nSummonMonster1=26 ; goblin\r\nSummonMonster2=32\r\n\r\n; Otra\r\nGreaterLifeConstA=1\r\n';
  const state = summons.parse(original);
  state.slots[0] = 7;
  state.constants.SummonMaxRate = 150;
  const out = summons.apply(original, state);
  assert.match(out, /SummonMonster1=7 ; goblin\r\n/);
  assert.match(out, /HealConstA=5\r\n/);
  assert.match(out, /GreaterLifeConstA=1\r\n/);
  // Las que faltaban quedan después de la última Summon*, antes del resto.
  assert.ok(out.indexOf('SummonMonster11=') > out.indexOf('SummonMonster2='));
  assert.ok(out.indexOf('SummonMaxRate=150') < out.indexOf('; Otra'));
  assert.equal(summons.apply(out, summons.parse(out)), out);
});

test('summons: validate rechaza monstruos que no existen y valores fuera de rango', () => {
  const state = summons.parse('');
  const ids = new Set(state.slots);
  assert.deepEqual(summons.validate(state, ids), state);
  assert.throws(() => summons.validate({ ...state, slots: [...state.slots.slice(0, 10), 9999] }, ids), summons.SummonError);
  assert.throws(() => summons.validate({ ...state, slots: [...state.slots.slice(0, 10), 999] }, ids), /no está en Monster.txt/);
  assert.throws(() => summons.validate({ ...state, constants: { ...state.constants, SummonMaxRate: -1 } }, ids), summons.SummonError);
});

test('summons: rate sigue la fórmula del GameServer', () => {
  const c = summons.CONST_DEFAULTS;
  assert.equal(summons.rate(c, 1000, 200), 150);   // 1000/20 + 200/2
  assert.equal(summons.rate(c, 5000, 400), 300);   // tope
  assert.equal(summons.rate({ ...c, SummonConstB: 0 }, 1000, 200), 100);
});
