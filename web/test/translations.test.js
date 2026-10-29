'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const {
  parseTranslations,
  parseReference,
  validateName,
  sameFormatSpecifiers,
  applyEdits,
  serializeTranslations,
  pathFor
} = require('../lib/translations');

const CLIENT = [
  '//Textos del cliente',
  '//Índice\tTexto',
  '//WindowTitle',
  '0\t"Mu Online"',
  '//ZoomLevel',
  '1\t"Zoom: %d"',
  'end',
  ''
].join('\r\n');

test('lee un Client.txt: encabezado, comentarios por entrada y fin de línea', () => {
  const parsed = parseTranslations(CLIENT, 'client');
  assert.equal(parsed.eol, '\r\n');
  assert.deepEqual(parsed.header, ['//Textos del cliente', '//Índice\tTexto', '//WindowTitle']);
  assert.deepEqual(parsed.entries.map((e) => [e.key, e.name]), [[[0], 'Mu Online'], [[1], 'Zoom: %d']]);
  assert.deepEqual(parsed.entries[1].comments, ['//ZoomLevel']);
  assert.equal(parsed.errors.length, 0);
});

test('un archivo vacío (solo comentarios) no tiene entradas y conserva el encabezado', () => {
  const parsed = parseTranslations('//Sección\tÍndice\tNombre\n//0\t0\t"Kris"\nend\n', 'items');
  assert.equal(parsed.entries.length, 0);
  assert.equal(parsed.header.length, 2);
  assert.equal(parsed.eol, '\n');
});

test('lee items con sección e índice y avisa de las líneas que no entiende', () => {
  const parsed = parseTranslations('0\t5\t"Espada"\nbasura\n1\t2\t"Escudo"\nend\n', 'items');
  assert.deepEqual(parsed.entries.map((e) => e.key), [[0, 5], [1, 2]]);
  assert.deepEqual(parsed.errors, [{ line: 2, text: 'basura' }]);
});

test('tolera el BOM del principio', () => {
  const parsed = parseTranslations('﻿7\t"Rayo"\nend\n', 'skills');
  assert.deepEqual(parsed.entries[0].key, [7]);
});

test('los nombres se miden en bytes UTF-8 y no en caracteres', () => {
  assert.ok(validateName('a'.repeat(63)).name);
  assert.match(validateName('a'.repeat(64)).error, /64 bytes/);
  // 32 letras de dos bytes = 64 bytes: entran 31 pero no 32.
  assert.ok(validateName('ñ'.repeat(31)).name);
  assert.match(validateName('ñ'.repeat(32)).error, /64 bytes/);
});

test('rechaza comillas, saltos de línea, tabulaciones, controles y vacío', () => {
  assert.ok(validateName('di"o').error);
  assert.ok(validateName('a\nb').error);
  assert.ok(validateName('a\tb').error);
  assert.ok(validateName('a\u0007b').error);
  assert.ok(validateName('a b').error);
  assert.ok(validateName('').error);
});

test('normaliza a NFC', () => {
  const decomposed = 'é';
  assert.equal(validateName(decomposed).name, 'é');
});

test('acepta texto no latino y emoji', () => {
  assert.equal(validateName('Владимир').name, 'Владимир');
  assert.equal(validateName('Espada 🗡️').name, 'Espada 🗡️');
});

test('los especificadores de formato tienen que ser los mismos y en el mismo orden', () => {
  assert.ok(sameFormatSpecifiers('Zoom: %d', 'Zoom: %d'));
  assert.ok(sameFormatSpecifiers('Opacidad: %d%%', 'Alpha: %d%%'));
  assert.ok(sameFormatSpecifiers('%.10s || %s: %u', '%.10s - %s: %u'));
  assert.ok(!sameFormatSpecifiers('Zoom: %d', 'Zoom: %s'));
  assert.ok(!sameFormatSpecifiers('Zoom: %d', 'Zoom'));
  assert.ok(!sameFormatSpecifiers('%s %d', '%d %s'));
});

test('con referencia, un formato distinto se rechaza', () => {
  assert.ok(validateName('Zoom: %s', { reference: 'Zoom: %d' }).error);
  assert.ok(validateName('Ampliación: %d', { reference: 'Zoom: %d' }).name);
});

test('aplicar cambios: agrega, modifica y quita, y deja todo ordenado', () => {
  const parsed = parseTranslations(CLIENT, 'client');
  const result = applyEdits(parsed, [
    { key: [5], name: 'Nuevo' },
    { key: [0], name: 'Mu Linux' },
    { key: [1], name: '' }
  ], 'client');
  assert.deepEqual(result.entries.map((e) => [e.key[0], e.name]), [[0, 'Mu Linux'], [5, 'Nuevo']]);
});

test('si un cambio falla no se aplica ninguno', () => {
  const parsed = parseTranslations(CLIENT, 'client');
  const result = applyEdits(parsed, [
    { key: [0], name: 'Bien' },
    { key: [1], name: 'Zoom: %s' }
  ], 'client', { referenceFor: (key) => (key[0] === 1 ? 'Zoom: %d' : undefined) });
  assert.equal(result.entries, undefined);
  assert.deepEqual(result.errors.map((e) => e.key), [[1]]);
});

test('claves fuera de rango se rechazan', () => {
  const parsed = parseTranslations('end\n', 'items');
  assert.ok(applyEdits(parsed, [{ key: [16, 0], name: 'x' }], 'items').errors);
  assert.ok(applyEdits(parsed, [{ key: [0, 512], name: 'x' }], 'items').errors);
  assert.ok(applyEdits(parsed, [{ key: [0], name: 'x' }], 'items').errors);
  assert.ok(applyEdits(parsed, [{ key: [0, 1], name: 'x' }], 'items').entries);
});

test('escribir y volver a leer devuelve lo mismo, con el fin de línea original', () => {
  const parsed = parseTranslations(CLIENT, 'client');
  const result = applyEdits(parsed, [{ key: [1], name: 'Ampliación: %d' }], 'client');
  const out = serializeTranslations(parsed, result.entries);
  assert.ok(out.endsWith('end\r\n'));
  assert.ok(!/[^\r]\n/.test(out));
  const back = parseTranslations(out, 'client');
  assert.deepEqual(back.entries.map((e) => [e.key, e.name]), [[[0], 'Mu Online'], [[1], 'Ampliación: %d']]);
  assert.deepEqual(back.entries[1].comments, ['//ZoomLevel']);
});

test('las referencias salen de los archivos de datos del server', () => {
  const items = parseReference('//cabecera\n0\n0\t0\t0\t1\t2\t"Kris"\t6\n1\t0\t0\t1\t3\t"Short Sword"\nend\n1\n0\t0\t0\t1\t2\t"Mace"\nend\n', 'items');
  assert.deepEqual(items, [
    { key: [0, 0], name: 'Kris' },
    { key: [0, 1], name: 'Short Sword' },
    { key: [1, 0], name: 'Mace' }
  ]);
  assert.deepEqual(parseReference('0\t1\t"Bull Fighter"\t6\n', 'monsters'), [{ key: [0], name: 'Bull Fighter' }]);
  assert.deepEqual(parseReference('1\t"Poison"\t12\n', 'skills'), [{ key: [1], name: 'Poison' }]);
  assert.deepEqual(parseReference('//ZoomLevel\n1\t"Zoom: %d"\nend\n', 'client'), [{ key: [1], name: 'Zoom: %d', label: 'ZoomLevel' }]);
});

test('solo se arman rutas de idiomas y tablas conocidos', () => {
  assert.equal(pathFor('Spn', 'client'), 'Data/Text/Spn/Client.txt');
  assert.throws(() => pathFor('../x', 'client'));
  assert.throws(() => pathFor('Spn', 'passwd'));
});
