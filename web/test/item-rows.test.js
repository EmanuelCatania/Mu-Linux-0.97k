'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const rows = require('../lib/item-rows');

const ITEM_TXT = fs.readFileSync(path.join(__dirname, '..', '..', 'MuServer', 'Data', 'Item', 'Item.txt'), 'utf8');

const sample = [
  '//Index\tSlot',
  '0',
  '0\t0\t0\t1\t2\t1\t1\t1\t"Kris"\t6\t6\t11\t50\t20\t0\t0\t0\t40\t40\t0\t0\t1\t1\t1\t1\t1',
  '1\t0\t0\t1\t3\t1\t1\t1\t"Short Sword"\t3\t3\t7\t20\t22\t0\t0\t0\t60\t0\t0\t0\t1\t1\t1\t1\t1',
  '5\t0\t0\t1\t3\t1\t1\t1\t"Blade"\t36\t36\t47\t30\t39\t0\t0\t0\t80\t50\t0\t0\t1\t1\t1\t1\t1',
  'end',
  '14',
  '0\t0\t0\t1\t1\t1\t1\t1\t"Apple"\t0\t0',
  'end',
  ''
].join('\r\n');

const newSword = (index) => ({
  key: [0, index],
  name: 'Espada nueva',
  slot: 0, skill: 0, width: 1, height: 3, serial: 1, option: 1, drop: 1,
  stats: Array(rows.statColumns(0).length).fill(0),
  tail: { behavior: [0, 1], folder: 'Item\\Custom\\20\\', model: 'Sword21', glow: [255, 40, 40], equippedFolder: null, equippedModel: null, gate: null }
});

test('las columnas por sección suman lo que el server espera', () => {
  const expected = { 0: 26, 1: 26, 5: 26, 6: 23, 7: 23, 9: 23, 10: 23, 11: 23, 12: 21, 13: 20, 14: 11, 15: 17 };
  for (const [section, count] of Object.entries(expected)) assert.equal(rows.baseCount(Number(section)), count, `sección ${section}`);
  assert.throws(() => rows.statColumns(16));
});

test('todas las filas reales de Item.txt se leen bien', () => {
  const file = rows.parseItemFile(ITEM_TXT);
  let total = 0;
  for (const section of Object.values(file.sections)) {
    for (const row of section.rows) {
      assert.ok(!row.malformed, `fila mal formada en la sección ${section.number}: ${row.line?.slice(0, 60)}`);
      assert.equal(row.stats.length, rows.statColumns(section.number).length);
      total += 1;
    }
  }
  assert.ok(total > 250, `se leyeron ${total} filas`);
  assert.equal(Object.keys(file.sections).length, 16);
});

test('reconstruir cada fila real da las mismas columnas que el original', () => {
  const file = rows.parseItemFile(ITEM_TXT);
  for (const section of Object.values(file.sections)) {
    for (const row of section.rows) {
      const rebuilt = rows.buildRowLine(rows.rowModel(section.number, row), row.comment);
      const a = rows.tokenize(row.line).tokens.map((t) => t.text);
      const b = rows.tokenize(rebuilt).tokens.map((t) => t.text);
      // '00,020' son dos columnas y se vuelve a escribir igual; solo puede cambiar
      // el relleno de los '*' del final, que no cuentan.
      while (a.length && a[a.length - 1] === '*') a.pop();
      assert.deepEqual(b.map((t) => t.replace(/^0+(?=\d)/, '')), a.map((t) => t.replace(/^0+(?=\d)/, '')), `${section.number},${row.index} ${row.name}`);
    }
  }
});

test('la coma separa columnas: "00,020" es el item 0, 20', () => {
  const { tokens } = rows.tokenize('32\t0\t22\t1\t4\t1\t1\t0\t"Crimson Knight Blade"\t1\t00,020\t"Item\\Custom\\20\\"\t"Sword21"\t255\t40\t40');
  assert.equal(tokens[8].text, 'Crimson Knight Blade');
  assert.deepEqual(tokens.slice(10, 12).map((t) => t.text), ['00', '020']);
  const tail = rows.parseTail(tokens.slice(10));
  assert.deepEqual(tail.behavior, [0, 20]);
  assert.equal(tail.folder, 'Item\\Custom\\20\\');
  assert.equal(tail.model, 'Sword21');
  assert.deepEqual(tail.glow, [255, 40, 40]);
  assert.equal(tail.gate, null);
});

test('las columnas opcionales con * se leen como "la original"', () => {
  const t = (text) => rows.tokenize(text).tokens;
  const tail = rows.parseTail(t('* * * * "Item\\Mount\\" "fenril" 22'));
  assert.equal(tail.behavior, null);
  assert.equal(tail.folder, null);
  assert.equal(tail.glow, null);
  assert.equal(tail.equippedFolder, 'Item\\Mount\\');
  assert.equal(tail.equippedModel, 'fenril');
  assert.equal(tail.gate, 22);
  assert.deepEqual(rows.tailTokens({ behavior: null, folder: null, model: null, glow: null, equippedFolder: null, equippedModel: null, gate: 22 }), ['*', '*', '*', '*', '*', '*', '22']);
  assert.deepEqual(rows.tailTokens({}), []);
});

test('agregar una fila la deja en su lugar, con el fin de línea del archivo', () => {
  const result = rows.addRow(sample, newSword(3));
  assert.equal(result.errors, undefined);
  const lines = result.content.split('\r\n');
  const order = lines.filter((l) => /^\d+\t/.test(l)).map((l) => l.split('\t')[0]);
  assert.deepEqual(order, ['0', '1', '3', '5', '0']);
  assert.ok(lines.includes('3\t0\t0\t1\t3\t1\t1\t1\t"Espada nueva"\t' + Array(rows.statColumns(0).length).fill(0).join('\t') + '\t00,001\t"Item\\Custom\\20\\"\t"Sword21"\t255\t40\t40'));
  assert.ok(result.content.endsWith('end\r\n'));
  assert.ok(!/[^\r]\n/.test(result.content));
  // el resto del archivo queda igual
  for (const line of sample.split('\r\n')) if (line) assert.ok(lines.includes(line), line.slice(0, 40));
});

test('agregar al final de una sección y a una sección sin filas de ese índice', () => {
  const end = rows.addRow(sample, newSword(40));
  assert.deepEqual(end.content.split('\r\n').filter((l) => /^\d+\t.*Espada/.test(l)).length, 1);
  const lines = end.content.split('\r\n');
  assert.ok(lines.indexOf(lines.find((l) => l.includes('Espada nueva'))) < lines.indexOf('end'));
  const first = rows.addRow(sample, { ...newSword(0), key: [0, 0] });
  assert.ok(first.errors[0].includes('ya existe'));
});

test('no deja agregar un item que ya existe ni uno sin Comportamiento si es agregado', () => {
  assert.ok(rows.addRow(sample, newSword(1)).errors[0].includes('ya existe'));
  const sinComportamiento = { ...newSword(40), tail: { behavior: null, folder: null, model: null, glow: null } };
  assert.ok(rows.addRow(sample, sinComportamiento).errors.some((e) => /Comportamiento/.test(e)));
  // un item original (índice < 32) puede no tener Comportamiento
  assert.equal(rows.addRow(sample, { ...newSword(2), tail: {} }).errors, undefined);
});

test('validaciones: nombre, números, rutas, brillo y carpeta con modelo', () => {
  const ok = newSword(40);
  assert.deepEqual(rows.validateRow(ok), []);
  assert.ok(rows.validateRow({ ...ok, name: 'a"b' }).some((e) => /comillas/.test(e)));
  assert.ok(rows.validateRow({ ...ok, name: 'x'.repeat(32) }).some((e) => /31 bytes/.test(e)));
  assert.ok(rows.validateRow({ ...ok, name: 'ñ'.repeat(16) }).some((e) => /31 bytes/.test(e)));
  assert.ok(rows.validateRow({ ...ok, width: 9 }).some((e) => /Ancho/.test(e)));
  assert.ok(rows.validateRow({ ...ok, stats: [1, 2] }).some((e) => /columnas de datos/.test(e)));
  assert.ok(rows.validateRow({ ...ok, stats: Array(rows.statColumns(0).length).fill(1.5) }).some((e) => /entero/.test(e)));
  assert.ok(rows.validateRow({ ...ok, tail: { ...ok.tail, folder: '../x' } }).some((e) => /ruta/.test(e)));
  assert.ok(rows.validateRow({ ...ok, tail: { ...ok.tail, glow: [256, 0, 0] } }).some((e) => /Brillo/.test(e)));
  assert.ok(rows.validateRow({ ...ok, tail: { ...ok.tail, model: null } }).some((e) => /juntos/.test(e)));
  assert.ok(rows.validateRow({ ...ok, key: [16, 1] }).length > 0);
  assert.ok(rows.validateRow({ ...ok, key: [0, 512] }).length > 0);
});

test('reemplazar una fila cambia solo esa línea', () => {
  const file = rows.parseItemFile(sample);
  const model = rows.rowModel(0, rows.findRow(file, [0, 1]));
  model.name = 'Espada corta';
  model.stats[0] = 9;
  const result = rows.replaceRow(sample, model);
  const before = sample.split('\r\n');
  const after = result.content.split('\r\n');
  assert.equal(after.length, before.length);
  const changed = after.filter((l, i) => l !== before[i]);
  assert.equal(changed.length, 1);
  assert.ok(changed[0].includes('"Espada corta"'));
  assert.ok(rows.replaceRow(sample, { ...model, key: [0, 9] }).errors[0].includes('no existe'));
});

test('primer índice libre de la sección', () => {
  const file = rows.parseItemFile(sample);
  assert.equal(rows.nextFreeIndex(file, 0), 32);
  assert.equal(rows.nextFreeIndex(file, 0, 0), 2);
});

test('con el Item.txt real: agregar y volver a leer', () => {
  const file = rows.parseItemFile(ITEM_TXT);
  const index = rows.nextFreeIndex(file, 0);
  const result = rows.addRow(ITEM_TXT, newSword(index));
  assert.equal(result.errors, undefined);
  const back = rows.parseItemFile(result.content);
  const added = rows.findRow(back, [0, index]);
  assert.equal(added.name, 'Espada nueva');
  assert.deepEqual(added.tail.behavior, [0, 1]);
  assert.deepEqual(added.tail.glow, [255, 40, 40]);
  // no cambió la cantidad de filas de ninguna otra sección
  for (const [number, section] of Object.entries(file.sections)) {
    assert.equal(back.sections[number].rows.length, section.rows.length + (Number(number) === 0 ? 1 : 0));
  }
});
