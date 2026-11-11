const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const spawns = require('../lib/spawns');
const maps = require('../lib/maps');
const drops = require('../lib/drops');

const DATA = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data');
const read = (file) => fs.readFileSync(file, 'utf8').replace(/\r\n/g, '\n');

// El repo trae los drops en TOML: Data/Items/Drops.toml y un Drops.toml por mapa.
function repoDropFiles() {
  const files = [{ path: drops.GLOBAL_FILE, content: read(path.join(DATA, 'Items', drops.DROP_FILE)), map: -1 }];
  for (const folder of fs.readdirSync(path.join(DATA, 'Maps')).sort()) {
    const file = path.join(DATA, 'Maps', folder, drops.DROP_FILE);
    if (fs.existsSync(file)) {
      files.push({ path: `${spawns.MAPS_DIR}/${folder}/${drops.DROP_FILE}`, content: read(file), map: spawns.folderMap(folder) });
    }
  }
  return files;
}

// Nombres de mapa de los Map.toml del repo, como los usa el panel.
function mapNames() {
  const files = [];
  for (const folder of fs.readdirSync(path.join(DATA, 'Maps'))) {
    const file = path.join(DATA, 'Maps', folder, maps.MAP_FILE);
    if (fs.existsSync(file)) files.push({ path: file, content: read(file), map: spawns.folderMap(folder) });
  }
  const names = new Map(maps.parseToml(files).map((m) => [m.index, m.name]));
  return (id) => names.get(id);
}

test('drops: los Drops.toml del repo son los que arma el panel desde el .txt', () => {
  const repoFiles = repoDropFiles();
  const folders = new Map(fs.readdirSync(path.join(DATA, 'Maps')).map((f) => [spawns.folderMap(f), f]));
  const text = drops.buildLegacy(drops.parseToml(repoFiles));
  const built = drops.buildToml(drops.parseLegacy(text), { mapName: mapNames(), folders });
  assert.deepEqual(Object.keys(built).sort(), repoFiles.map((f) => f.path).sort());
  for (const f of repoFiles) assert.equal(built[f.path], f.content, f.path);
});

test('drops: un drop con mapa va al archivo de ese mapa', () => {
  const list = drops.parseLegacy('14,022\t0\t0\t*\t3\t5\t1\t0\t3\t*\t30\t80\t500\t\t//Jewel of Creation\nend\n');
  const files = drops.buildToml(list, { mapName: () => 'Noria' });
  const file = `${spawns.MAPS_DIR}/03-Noria/${drops.DROP_FILE}`;
  assert.deepEqual(Object.keys(files), [file]);
  assert.match(files[file], /^ {2}\{ item = \[14, 22\], options = \{ skill = 3, luck = 5, add = 1, exc = 0 \}, monster_level = \[30, 80\], rate = 500 \}, # Jewel of Creation$/m);
  const back = drops.parseToml([{ path: file, content: files[file], map: 3 }]);
  assert.equal(drops.buildLegacy(back), drops.buildLegacy(list));
});

test('drops: errores con el archivo y el campo', () => {
  const file = (body) => [{ path: 'Data/Maps/03-Noria/Drops.toml', content: `drop = [\n  ${body}\n]\n`, map: 3 }];
  assert.throws(() => drops.parseToml(file('{ item = [14, 13], rate = 100, chance = 2 },')), /campo desconocido 'chance'/);
  assert.throws(() => drops.parseToml(file('{ item = [14, 13] },')), /falta 'rate'/);
  assert.throws(() => drops.parseToml(file('{ item = [16, 13], rate = 100 },')), /sección del item/);
  assert.throws(() => drops.parseToml(file('{ item = [14, 13], options = { lucky = 1 }, rate = 100 },')), /opción desconocida 'lucky'/);
  assert.throws(() => drops.parseToml(file('{ item = [14, 13], monster_level = [5], rate = 100 },')), /'monster_level' es \[a, b\]/);
  assert.throws(() => drops.parseLegacy('14,013\t0\t0\n'), /14 columnas/);
});
