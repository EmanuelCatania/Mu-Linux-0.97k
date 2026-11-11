const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const spawns = require('../lib/spawns');
const maps = require('../lib/maps');
const gates = require('../lib/gates');

const DATA = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data');
const read = (file) => fs.readFileSync(file, 'utf8').replace(/\r\n/g, '\n');

// Los archivos de un tipo en las carpetas de mapa del repo.
function repoFiles(name) {
  const files = [];
  for (const folder of fs.readdirSync(path.join(DATA, 'Maps')).sort()) {
    const file = path.join(DATA, 'Maps', folder, name);
    if (fs.existsSync(file)) files.push({ path: `${spawns.MAPS_DIR}/${folder}/${name}`, content: read(file), map: spawns.folderMap(folder) });
  }
  return files;
}

test('gates: los Gates.toml del repo son los que arma el panel desde el .txt', () => {
  const files = repoFiles(gates.GATE_FILE);
  assert.ok(files.length > 0);
  const names = new Map(maps.parseToml(repoFiles(maps.MAP_FILE)).map((m) => [m.index, m.name]));
  const folders = new Map(fs.readdirSync(path.join(DATA, 'Maps')).map((f) => [spawns.folderMap(f), f]));
  const text = gates.buildLegacy(gates.parseToml(files));
  const built = gates.buildToml(gates.parseLegacy(text), { mapName: (id) => names.get(id), folders });
  assert.deepEqual(Object.keys(built).sort(), files.map((f) => f.path).sort());
  for (const f of files) assert.equal(built[f.path], f.content, f.path);
});

test('gates: una gate va al archivo de su mapa y conserva su número', () => {
  const list = gates.parseLegacy('7\t1\t3\t10\t20\t12\t22\t8\t0\t50\t400\t*\t*\t0\n8\t2\t0\t1\t2\t3\t4\t0\t5\t50\t400\t2\t9\t1\nend\n');
  const files = gates.buildToml(list, { mapName: (id) => ['Lorencia', '', '', 'Noria'][id] });
  assert.deepEqual(Object.keys(files).sort(), [`${spawns.MAPS_DIR}/00-Lorencia/${gates.GATE_FILE}`, `${spawns.MAPS_DIR}/03-Noria/${gates.GATE_FILE}`]);
  assert.match(files[`${spawns.MAPS_DIR}/03-Noria/${gates.GATE_FILE}`], /\{ index = 7, type = "entrance", area = \[10, 20, 12, 22\], target = 8, level = \[50, 400\] \}/);
  assert.match(files[`${spawns.MAPS_DIR}/00-Lorencia/${gates.GATE_FILE}`], /\{ index = 8, type = "exit", area = \[1, 2, 3, 4\], dir = 5, level = \[50, 400\], reset = \[2, 9\], account = 1 \}/);
  const back = gates.parseToml(Object.entries(files).map(([p, content]) => ({ path: p, content, map: spawns.folderMap(p.split('/')[2]) })));
  assert.equal(gates.buildLegacy(back), gates.buildLegacy(list));
});

test('gates: errores con el archivo y el campo', () => {
  const file = (body, map = 0) => ({ path: `Data/Maps/0${map}-X/Gates.toml`, content: `gate = [\n  ${body}\n]\n`, map });
  assert.throws(() => gates.parseToml([file('{ index = 1, type = "entrance", area = [1, 2, 3, 4], to = 2 },')]), /campo desconocido 'to'/);
  assert.throws(() => gates.parseToml([file('{ type = "exit", area = [1, 2, 3, 4] },')]), /falta 'index'/);
  assert.throws(() => gates.parseToml([file('{ index = 1, type = "portal", area = [1, 2, 3, 4] },')]), /falta 'type'/);
  assert.throws(() => gates.parseToml([file('{ index = 1, type = "exit", area = [1, 2, 3] },')]), /'area' es una lista de 4/);
  assert.throws(() => gates.parseToml([file('{ index = 5, type = "exit", area = [1, 2, 3, 4] },'), file('{ index = 5, type = "exit", area = [1, 2, 3, 4] },', 3)]), /la gate 5 ya está/);
  assert.throws(() => gates.parseLegacy('1\t3\t0\t1\t2\t3\t4\t0\t0\t0\t400\t*\t*\t0\n'), /Flag/);
});
