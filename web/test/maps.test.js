const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const spawns = require('../lib/spawns');
const maps = require('../lib/maps');

const DATA = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data');

// El repo trae las reglas de los mapas en TOML (Data/Maps/NN-Nombre/Map.toml).
function repoMapFiles() {
  const files = [];
  for (const folder of fs.readdirSync(path.join(DATA, 'Maps')).sort()) {
    const file = path.join(DATA, 'Maps', folder, maps.MAP_FILE);
    if (fs.existsSync(file)) {
      files.push({ path: `${spawns.MAPS_DIR}/${folder}/${maps.MAP_FILE}`, content: fs.readFileSync(file, 'utf8'), map: spawns.folderMap(folder) });
    }
  }
  return files;
}
const repoFiles = repoMapFiles();
const folders = new Map(repoFiles.map((f) => [f.map, f.path.split('/')[2]]));

test('maps: los Map.toml del repo son los que arma el panel desde el .txt', () => {
  assert.ok(repoFiles.length > 0);
  const legacy = maps.buildLegacy(maps.parseToml(repoFiles));
  const built = maps.buildToml(maps.parseLegacy(legacy), { folders });
  assert.deepEqual(Object.keys(built).sort(), repoFiles.map((f) => f.path).sort());
  for (const f of repoFiles) assert.equal(built[f.path], f.content, f.path);
});

test('maps: * en el .txt es una clave que falta en el TOML', () => {
  const [d] = maps.parseLegacy('0\t*\t*\t100\t*\t0\t*\t1\t0\t"Lorencia"\nend\n');
  assert.equal(d.nonPK, -1);
  assert.equal(d.viewRange, -1);
  const text = maps.buildToml([d])[`${spawns.MAPS_DIR}/00-Lorencia/${maps.MAP_FILE}`];
  assert.ok(!/^pk =/m.test(text) && !/^view_range =/m.test(text) && !/^dead_gate =/m.test(text));
  assert.match(text, /^respawn_in_place = true$/m);
  const [back] = maps.parseToml([{ path: 'x', content: text, map: 0 }]);
  assert.equal(maps.buildLegacy([back]), maps.buildLegacy([d]));
});

test('maps: errores con el archivo y el campo', () => {
  const file = (body) => [{ path: 'Data/Maps/00-Lorencia/Map.toml', content: body, map: 0 }];
  assert.throws(() => maps.parseToml(file('name = "L"\npvp = 1\n')), /campo desconocido 'pvp'/);
  assert.throws(() => maps.parseToml(file('name = "L"\npk = "on"\n')), /'pk' vale/);
  assert.throws(() => maps.parseToml(file('name = "L"\nview_range = 300\n')), /'view_range' tiene que ser un entero entre 0 y 255/);
  assert.throws(() => maps.parseToml(file('view_range = 15\n')), /falta 'name'/);
  assert.throws(() => maps.parseLegacy('0\t*\t15\t"Lorencia"\n'), /10 columnas/);
  assert.throws(() => maps.parseLegacy('0\t3\t15\t100\t100\t0\t17\t0\t1\t"L"\n'), /NonPK/);
});
