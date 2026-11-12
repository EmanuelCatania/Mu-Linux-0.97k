'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const x = require('../lib/item-extras');
const { validateItemJson, validatePetJson } = require('../lib/custom-items');

const ITEMS = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data', 'Items');

function loadRepo() {
  const files = {};
  const sections = {};
  for (const name of fs.readdirSync(ITEMS)) {
    const section = x.fileSection(name);
    if (section < 0) continue;
    files[section] = fs.readFileSync(path.join(ITEMS, name), 'utf8');
    sections[section] = x.parseSection(files[section], name);
  }
  return { files, sections };
}

test('nombres de archivo de sección', () => {
  assert.equal(x.sectionFileName(0), '00-swords.toml');
  assert.equal(x.sectionFileName(13), '13-helpers.toml');
  assert.equal(x.sectionPath(4), 'Data/Items/04-bows.toml');
  assert.equal(x.fileSection('Data/Items/12-wings.toml'), 12);
  assert.equal(x.fileSection('12-cualquier-nombre.toml'), 12);
  assert.equal(x.fileSection('Drops.toml'), -1);
  assert.equal(x.fileSection('16-x.toml'), -1);
});

test('los TOML del repo se leen y lo de adentro valida con las reglas del server', () => {
  const { sections } = loadRepo();
  assert.ok(Object.keys(sections).length >= 15);
  for (const [section, items] of Object.entries(sections)) {
    for (const [index, item] of Object.entries(items)) {
      const key = `${section},${index}`;
      if (item.inventory || item.effects || item.tooltip) {
        assert.deepEqual(validateItemJson({ item: key, inventory: item.inventory, effects: item.effects, tooltip: item.tooltip }), [], key);
      }
      if (item.pet) assert.deepEqual(validatePetJson({ item: key, ...item.pet }, { itemSlot: 8 }), [], key);
    }
  }
  assert.deepEqual(sections[13][400].tooltip, [{ color: 'gold', text: 'Zen picker' }]);
  assert.equal(sections[13][400].pet.movement.type, 'orbit');
  assert.equal(sections[14][1].stack, 255);
});

test('reescribir un item sin cambios deja el archivo igual', () => {
  const { files, sections } = loadRepo();
  for (const [section, items] of Object.entries(sections)) {
    for (const [index, item] of Object.entries(items)) {
      assert.equal(x.replaceItem(files[section], Number(section), Number(index), item), files[section], `${section},${index}`);
    }
  }
});

test('la tabla de precio y apilado ida y vuelta no cambia ningún archivo', () => {
  const { files, sections } = loadRepo();
  for (const kind of ['value', 'stack']) {
    const rows = x.buildRows(kind, sections);
    assert.deepEqual(x.applyRows(kind, x.parseRows(kind, rows), files), {}, kind);
  }
});

test('agregar, cambiar y borrar items', () => {
  const base = x.replaceItem('', 5, 7, { glow: [1, 2, 3] }, 'Staff X');
  assert.match(base, /^# Items de la sección 5/);
  assert.match(base, /# Staff X\n\[007\]\nglow = \[1, 2, 3\]/);

  let text = x.replaceItem(base, 5, 3, { stack: 10 }, 'Antes');
  assert.deepEqual(Object.keys(x.parseSection(text)), ['3', '7']);
  assert.ok(text.indexOf('[003]') < text.indexOf('[007]'), 'en orden de índice');

  text = x.replaceItem(text, 5, 7, { glow: [9, 9, 9], value: [{ level: 0, buy: 100 }] });
  assert.deepEqual(x.parseSection(text)[7], { glow: [9, 9, 9], value: [{ level: 0, buy: 100 }] });
  assert.match(text, /# Staff X\n\[007\]/, 'conserva el comentario del item');

  text = x.replaceItem(text, 5, 3, null);
  assert.deepEqual(Object.keys(x.parseSection(text)), ['7']);
});

test('la tabla de precios se aplica a la sección que corresponde', () => {
  const files = { 14: x.replaceItem('', 14, 1, { stack: 255 }) };
  const changed = x.applyRows('value', x.parseRows('value', '14,001\t*\t20\t*\n13,016\t1\t15000\t*\nend\n'), files);
  assert.deepEqual(x.parseSection(changed[14])[1], { stack: 255, value: [{ buy: 20 }] });
  assert.deepEqual(x.parseSection(changed[13])[16], { value: [{ level: 1, buy: 15000 }] });
});

test('rechaza claves desconocidas y valores fuera de rango', () => {
  assert.throws(() => x.parseSection('[001]\ncolor = 1\n'), /clave desconocida/);
  assert.throws(() => x.parseSection('[001]\nglow = [1, 2, 300]\n'), /glow/);
  assert.throws(() => x.parseSection('[001]\nbow = "lanza"\n'), /bow/);
  assert.throws(() => x.parseSection('[001]\nstack = 300\n'), /stack/);
  assert.throws(() => x.parseSection('[600]\nglow = [1, 2, 3]\n'), /índice/);
  assert.throws(() => x.parseSection('[001]\nmodel = { folder = "Item/Custom/con/una/carpeta/demasiado/larga/", name = "x" }\n'), /40 caracteres/);
  assert.throws(() => x.parseRows('value', '14,001\t20\t5\t*\n'), /nivel/);
});
