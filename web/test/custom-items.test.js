'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {
  parseKey,
  fileName,
  pathFor,
  keyFromFileName,
  parseJsonFile,
  serializeJsonFile,
  validateItemJson,
  validatePetJson,
  parseItemList
} = require('../lib/custom-items');

const DATA = path.join(__dirname, '..', '..', 'MuServer', 'Data');
const read = (relative) => fs.readFileSync(path.join(DATA, relative), 'utf8');

test('claves: acepta "12,32", "12,032" y [12, 32]; rechaza lo demás', () => {
  assert.deepEqual(parseKey('12,32'), [12, 32]);
  assert.deepEqual(parseKey('12,032'), [12, 32]);
  assert.deepEqual(parseKey([13, 400]), [13, 400]);
  assert.equal(parseKey('16,0'), null);
  assert.equal(parseKey('0,512'), null);
  assert.equal(parseKey('x'), null);
  assert.equal(parseKey(null), null);
});

test('nombres de archivo: SS_III.json y de vuelta', () => {
  assert.equal(fileName([3, 0]), '03_000.json');
  assert.equal(fileName([13, 400]), '13_400.json');
  assert.equal(pathFor('pets', [13, 400]), 'Data/Custom/Pets/13_400.json');
  assert.deepEqual(keyFromFileName('05_032.json'), [5, 32]);
  assert.equal(keyFromFileName('5_32.json'), null);
  assert.equal(keyFromFileName('../x.json'), null);
  assert.throws(() => pathFor('otro', [1, 1]));
  assert.throws(() => pathFor('items', [99, 1]));
});

test('los JSON de ejemplo del repo se leen y validan sin errores', () => {
  for (const file of ['Custom/Items/03_000.json', 'Custom/Items/13_400.json']) {
    const parsed = parseJsonFile(read(file));
    assert.equal(parsed.error, undefined, file);
    assert.ok(parsed.header.length > 0, `${file} conserva su encabezado`);
    assert.deepEqual(validateItemJson(parsed.data), [], file);
  }
  const pet = parseJsonFile(read('Custom/Pets/13_400.json'));
  assert.deepEqual(validatePetJson(pet.data, { itemSlot: 8 }), []);
});

test('leer y escribir un JSON de ejemplo conserva el contenido y el encabezado', () => {
  for (const [file, validate] of [['Custom/Items/13_400.json', validateItemJson], ['Custom/Pets/13_400.json', validatePetJson]]) {
    const parsed = parseJsonFile(read(file));
    const out = serializeJsonFile(parsed.header, parsed.data);
    const back = parseJsonFile(out);
    assert.deepEqual(back.data, parsed.data, file);
    assert.deepEqual(back.header, parsed.header, file);
    assert.deepEqual(validate(back.data), [], file);
    assert.ok(out.endsWith('}\r\n'));
  }
});

test('avisa de los comentarios que están adentro del JSON y se perderían', () => {
  const withInner = parseJsonFile('// cabecera\n{\n  // adentro\n  "item": "1,1"\n}\n');
  assert.equal(withInner.hasInnerComments, true);
  assert.deepEqual(withInner.header, ['// cabecera']);
  const plain = parseJsonFile('// cabecera\n{ "item": "1,1" }\n');
  assert.equal(plain.hasInnerComments, false);
  assert.ok(parseJsonFile('{ roto').error);
});

test('los // dentro de una cadena no son comentarios', () => {
  const parsed = parseJsonFile('{ "item": "1,1", "tooltip": ["http://x.com"] }');
  assert.deepEqual(parsed.data.tooltip, ['http://x.com']);
});

test('item: un efecto de sprite necesita textura o bitmap y huesos', () => {
  const base = { item: '12,32' };
  assert.deepEqual(validateItemJson({ ...base, effects: [{ type: 'sprite', texture: 'Effect/Flare.jpg', bones: [5, 6] }] }), []);
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'sprite', bones: [5] }] }).some((e) => /texture/.test(e)));
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'sprite', texture: 'a.jpg', bones: [] }] }).some((e) => /bones/.test(e)));
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'sprite', texture: 'a.jpg', bones: [200] }] }).some((e) => /hueso/.test(e)));
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'sprite', texture: 'a.jpg', bones: Array(33).fill(1) }] }).some((e) => /bones/.test(e)));
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'laser', texture: 'a.jpg', bones: [1] }] }).some((e) => /type/.test(e)));
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'particle', bones: [1] }] }).some((e) => /particle/.test(e)));
  assert.deepEqual(validateItemJson({ ...base, effects: [{ type: 'particle', particle: 1230, bones: [13] }] }), []);
});

test('item: rutas de textura inseguras y vectores mal formados', () => {
  const base = { item: '12,32' };
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'sprite', texture: '../x.jpg', bones: [1] }] }).some((e) => /textura/.test(e)));
  assert.ok(validateItemJson({ ...base, effects: [{ type: 'sprite', texture: 'a.jpg', bones: [1], color: [1, 0] }] }).some((e) => /color/.test(e)));
  assert.ok(validateItemJson({ ...base, inventory: { position: [0, 0, 0] } }).some((e) => /position/.test(e)));
  assert.ok(validateItemJson({ ...base, inventory: { rotation: ['a', 0, 0] } }).some((e) => /rotation/.test(e)));
  assert.deepEqual(validateItemJson({ ...base, inventory: { position: [0, 0.05], rotation: [0, 90, 20], scale: 0.5 } }), []);
});

test('item: tooltip de hasta 6 líneas, 59 bytes y colores conocidos', () => {
  const base = { item: '13,400' };
  assert.deepEqual(validateItemJson({ ...base, tooltip: ['a', { text: 'b', color: 'gold' }] }), []);
  assert.ok(validateItemJson({ ...base, tooltip: Array(7).fill('x') }).length > 0);
  assert.ok(validateItemJson({ ...base, tooltip: ['a'.repeat(60)] }).some((e) => /59 bytes/.test(e)));
  assert.deepEqual(validateItemJson({ ...base, tooltip: ['a'.repeat(59)] }), []);
  // los acentos cuentan en bytes
  assert.ok(validateItemJson({ ...base, tooltip: ['ñ'.repeat(30)] }).some((e) => /bytes/.test(e)));
  assert.ok(validateItemJson({ ...base, tooltip: [{ text: 'a', color: 'rosa' }] }).some((e) => /color/.test(e)));
  assert.ok(validateItemJson({ ...base, tooltip: [''] }).some((e) => /texto/.test(e)));
});

test('item: sin la clave "item" no vale', () => {
  assert.ok(validateItemJson({}).length > 0);
  assert.ok(validateItemJson([]).length > 0);
  assert.ok(validateItemJson({ item: '99,1' }).length > 0);
});

test('pet: movimiento, período y habilidad de levantar zen', () => {
  const pet = { item: '13,400', movement: { type: 'orbit', radius: 50, period: 4000, height: 20 }, abilities: [{ type: 'pickup', what: 'zen', range: 3 }] };
  assert.deepEqual(validatePetJson(pet, { itemSlot: 8 }), []);
  assert.ok(validatePetJson({ ...pet, movement: undefined }).some((e) => /movement/.test(e)));
  assert.ok(validatePetJson({ ...pet, movement: { type: 'saltar' } }).some((e) => /movement\.type/.test(e)));
  assert.ok(validatePetJson({ ...pet, movement: { type: 'follow', period: 50 } }).some((e) => /100 ms/.test(e)));
  assert.ok(validatePetJson({ ...pet, abilities: [{ type: 'pickup', range: 9 }] }).some((e) => /range/.test(e)));
  assert.ok(validatePetJson({ ...pet, abilities: [{ type: 'pickup', what: 'items' }] }).some((e) => /zen/.test(e)));
  assert.ok(validatePetJson({ ...pet, abilities: [{ type: 'curar' }] }).some((e) => /desconocida/.test(e)));
  assert.ok(validatePetJson(pet, { itemSlot: 2 }).some((e) => /slot del helper/.test(e)));
});

test('la lista de items sale de Item.txt, con sus índices agregados', () => {
  const list = parseItemList(read('Item/Item.txt'));
  assert.ok(list.length > 200);
  const kris = list.find((i) => i.key[0] === 0 && i.key[1] === 0);
  assert.equal(kris.name, 'Kris');
  const added = list.find((i) => i.key[0] === 5 && i.key[1] === 32);
  if (added) assert.ok(added.name.length > 0);
  assert.ok(list.every((i) => i.key[0] >= 0 && i.key[0] <= 15));
});
