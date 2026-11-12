const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const spawns = require('../lib/spawns');

const DATA = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data');
// El repo trae los spawns en TOML (Data/Maps y Data/Events/Invasions). Los
// tests parten del .txt que sale de ellos, que es el mismo MonsterSetBase.txt
// que había antes de la conversión.
function repoTomlFiles() {
  const files = [];
  for (const folder of fs.readdirSync(path.join(DATA, 'Maps')).sort()) {
    const file = path.join(DATA, 'Maps', folder, 'Spawns.toml');
    if (fs.existsSync(file)) {
      files.push({ path: `${spawns.MAPS_DIR}/${folder}/Spawns.toml`, content: fs.readFileSync(file, 'utf8'), map: spawns.folderMap(folder) });
    }
  }
  files.push({ path: spawns.INVASIONS_FILE, content: fs.readFileSync(path.join(DATA, 'Events', 'Invasions', 'Spawns.toml'), 'utf8'), map: null });
  return files;
}
const repoFiles = repoTomlFiles();
const legacyText = spawns.buildLegacy(spawns.parseToml(repoFiles), () => '');

test('spawns: los TOML del repo son los que arma el panel desde el .txt', () => {
  const folders = new Map(repoFiles.filter((f) => f.map !== null).map((f) => [f.map, f.path.split('/')[2]]));
  const built = spawns.buildToml(spawns.parseLegacy(legacyText), {
    mapName: (id) => folders.has(id) ? folders.get(id).replace(/^\d+-/, '') : `Map${id}`,
    folders
  });
  // Los comentarios con el nombre del monstruo salen de Monster.txt: se comparan
  // las entradas, no el texto.
  for (const f of repoFiles) {
    if (f.map === null) continue;
    assert.ok(built[f.path], `falta ${f.path}`);
    const again = spawns.parseToml([{ ...f, content: built[f.path] }]);
    assert.deepEqual(again.map(fields), spawns.parseToml([f]).map(fields), f.path);
  }
});

const fields = (e) => [e.section, e.monster, e.map, e.dis, e.x, e.y, e.tx, e.ty, e.dir, e.count, e.value];
const bySectionAndMap = (entries) => entries
  .map((e, order) => ({ e, order }))
  .sort((a, b) => (a.e.section - b.e.section) || (a.e.map - b.e.map) || (a.order - b.order))
  .map(({ e }) => e);

function tomlFiles(files) {
  return Object.entries(files).map(([file, content]) => ({
    path: file,
    content,
    map: file === spawns.INVASIONS_FILE ? null : spawns.folderMap(file.split('/')[2])
  }));
}

test('spawns: el .txt del repo pasa a TOML y vuelve igual', () => {
  const legacy = spawns.parseLegacy(legacyText);
  assert.equal(legacy.length, 2882);

  const files = spawns.buildToml(legacy, { mapName: (id) => `Map${id}` });
  assert.ok(files[spawns.INVASIONS_FILE], 'las invasiones van a su propio archivo');
  assert.ok(!Object.keys(files).some((f) => f.includes('/Maps/') && files[f].includes('invasion =')));

  const back = spawns.parseToml(tomlFiles(files));
  assert.deepEqual(back.map(fields), bySectionAndMap(legacy).map(fields));

  // Y el texto que recibe el editor vuelve a dar la misma tabla.
  const again = spawns.parseLegacy(spawns.buildLegacy(back, () => ''));
  assert.deepEqual(again.map(fields), back.map(fields));
});

test('spawns: errores con archivo, sección y campo', () => {
  const file = (content, map = 0) => [{ path: 'Data/Maps/00-Lorencia/Spawns.toml', map, content }];
  assert.throws(() => spawns.parseToml(file('npc = [ { monster = 1, at = [1, 2], dirr = 3 } ]')), /campo desconocido 'dirr'/);
  assert.throws(() => spawns.parseToml(file('event_area = []')), /Events\/Invasions/);
  assert.throws(() => spawns.parseToml(file('area = [ { monster = 1, box = [1, 2, 3, 4] } ]')), /falta 'count'/);
  assert.throws(() => spawns.parseToml(file('npc = [ { monster = 1, at = [1, 300] } ]')), /0 a 255/);
  assert.throws(() => spawns.parseToml([{ path: spawns.INVASIONS_FILE, map: null, content: 'invasion = [ { monster = 1, box = [1, 2, 3, 4], count = 1 } ]' }]), /falta 'map'/);
  assert.throws(() => spawns.parseLegacy('1\n3 0 0 10 10 20'), /termina en medio/);
});

test('spawns: respeta el nombre de una carpeta que ya existe', () => {
  const entries = [{ section: 0, monster: 230, map: 0, dis: 0, x: 62, y: 130, tx: 0, ty: 0, dir: 3, count: 1, value: 0, comment: '' }];
  const files = spawns.buildToml(entries, { mapName: () => 'Lorencia', folders: new Map([[0, '00-MiLorencia']]) });
  assert.deepEqual(Object.keys(files), ['Data/Maps/00-MiLorencia/Spawns.toml']);
});
