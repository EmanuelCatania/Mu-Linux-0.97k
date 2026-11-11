const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const mapFolders = require('../lib/map-folders');

const DATA = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data');

// Copia Data/Maps y Data/Items a una carpeta temporal y arma el `io` sobre ella.
function fixture() {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'map-folders-'));
  for (const dir of ['Maps', 'Items']) fs.cpSync(path.join(DATA, dir), path.join(root, 'Data', dir), { recursive: true });
  const abs = (rel) => path.join(root, rel);
  const io = {
    list: async (rel) => {
      if (!fs.existsSync(abs(rel))) return null;
      return fs.readdirSync(abs(rel), { withFileTypes: true }).map((e) => ({ name: e.name, type: e.isDirectory() ? 'dir' : 'file' }));
    },
    read: async (rel) => fs.readFileSync(abs(rel), 'utf8').replace(/\r\n/g, '\n'),
    write: async (rel, text) => {
      const unchanged = fs.existsSync(abs(rel)) && fs.readFileSync(abs(rel), 'utf8') === text;
      if (!unchanged) fs.writeFileSync(abs(rel), text);
      return { unchanged };
    },
    remove: async (rel) => {
      fs.unlinkSync(abs(rel));
      return {};
    }
  };
  return { root, abs, editor: mapFolders.create(io), done: () => fs.rmSync(root, { recursive: true, force: true }) };
}

test('map-folders: lista las carpetas con sus archivos y qué tipos ya están en TOML', async () => {
  const f = fixture();
  try {
    const { folders, kinds } = await f.editor.summary();
    const lore = folders.find((x) => x.map === 0);
    assert.equal(lore.folder, '00-Lorencia');
    assert.ok(lore.kinds.includes('map') && lore.kinds.includes('spawns') && lore.kinds.includes('zones'));
    for (const k of Object.keys(mapFolders.KINDS)) assert.equal(kinds[k].toml, true, k);
    assert.equal(kinds.spawns.reload, 'spawns');
    assert.equal(kinds.spawns.deletable, false);
  } finally {
    f.done();
  }
});

test('map-folders: guarda sólo el archivo del mapa y devuelve qué recargar', async () => {
  const f = fixture();
  try {
    const { content } = await f.editor.read('02-Devias', 'gates');
    const otherBefore = fs.readFileSync(f.abs('Data/Maps/00-Lorencia/Gates.toml'), 'utf8');
    const edited = content.replace('dir = 7, level = [15, 400]', 'dir = 6, level = [15, 400]');
    assert.notEqual(edited, content);
    const saved = await f.editor.save('02-Devias', 'gates', edited);
    assert.equal(saved.reload, 'move');
    assert.equal(saved.unchanged, false);
    assert.equal(fs.readFileSync(f.abs('Data/Maps/02-Devias/Gates.toml'), 'utf8'), edited);
    assert.equal(fs.readFileSync(f.abs('Data/Maps/00-Lorencia/Gates.toml'), 'utf8'), otherBefore);
    assert.equal((await f.editor.save('02-Devias', 'gates', edited)).unchanged, true);
  } finally {
    f.done();
  }
});

test('map-folders: valida contra los otros mapas y no escribe si falla', async () => {
  const f = fixture();
  try {
    const before = fs.readFileSync(f.abs('Data/Maps/02-Devias/Gates.toml'), 'utf8');
    // La gate 1 ya existe en otro mapa: el número no se puede repetir.
    const dup = before.replace('{ index = 19,', '{ index = 1,');
    await assert.rejects(f.editor.save('02-Devias', 'gates', dup), (err) => mapFolders.errorStatus(err) === 400);
    await assert.rejects(f.editor.save('02-Devias', 'gates', dup, true));
    await assert.rejects(f.editor.save('02-Devias', 'zones', 'pvp = []\n'), /sección desconocida/);
    await assert.rejects(f.editor.save('00-Lorencia', 'map', 'name = ""\n'), /name/);
    assert.equal(fs.readFileSync(f.abs('Data/Maps/02-Devias/Gates.toml'), 'utf8'), before);
    assert.deepEqual(await f.editor.save('02-Devias', 'gates', before, true), { ok: true, valid: true });
  } finally {
    f.done();
  }
});

test('map-folders: un archivo nuevo sale con la explicación de los campos y se crea al guardar', async () => {
  const f = fixture();
  try {
    const { folders } = await f.editor.summary();
    const target = folders.find((x) => !x.kinds.includes('zones'));
    const read = await f.editor.read(target.folder, 'zones');
    assert.equal(read.exists, false);
    assert.match(read.content, new RegExp(`^# ${target.folder.replace(/^\d+-/, '')} \\(mapa ${target.map}\\)\n#`));
    const saved = await f.editor.save(target.folder, 'zones', read.content + 'safe = [\n  [1, 2, 3, 4],\n]\n');
    assert.equal(saved.reload, 'custom');
    assert.ok(fs.existsSync(f.abs(`Data/Maps/${target.folder}/Zones.toml`)));
    assert.equal((await f.editor.read(target.folder, 'zones')).exists, true);
  } finally {
    f.done();
  }
});

test('map-folders: borrar sí, salvo los spawns y el último archivo de un tipo', async () => {
  const f = fixture();
  try {
    await assert.rejects(f.editor.remove('00-Lorencia', 'spawns'), /no se puede borrar/);
    const { folders } = await f.editor.summary();
    const withZones = folders.filter((x) => x.kinds.includes('zones'));
    for (const z of withZones.slice(1)) await f.editor.remove(z.folder, 'zones');
    await assert.rejects(f.editor.remove(withZones[0].folder, 'zones'), (err) => err.status === 409);
    assert.ok(fs.existsSync(f.abs(`Data/Maps/${withZones[0].folder}/Zones.toml`)));
  } finally {
    f.done();
  }
});

test('map-folders: no crea el primer archivo de un tipo que sigue en .txt', async () => {
  const f = fixture();
  try {
    for (const x of (await f.editor.summary()).folders) {
      if (x.kinds.includes('gates')) fs.unlinkSync(f.abs(`Data/Maps/${x.folder}/Gates.toml`));
    }
    assert.equal((await f.editor.summary()).kinds.gates.toml, false);
    await assert.rejects(f.editor.save('00-Lorencia', 'gates', ''), (err) => err.status === 409);
    await assert.rejects(f.editor.read('99-Nada', 'gates'), (err) => err.status === 404);
    await assert.rejects(f.editor.read('00-Lorencia', 'otro'), /desconocido/);
  } finally {
    f.done();
  }
});
