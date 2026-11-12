const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const spawns = require('../lib/spawns');
const maps = require('../lib/maps');
const zones = require('../lib/zones');

const DATA = path.join(__dirname, '..', '..', 'server', 'MuServer', 'Data');
const read = (file) => fs.readFileSync(file, 'utf8').replace(/\r\n/g, '\n');

function repoFiles(name) {
  const files = [];
  for (const folder of fs.readdirSync(path.join(DATA, 'Maps')).sort()) {
    const file = path.join(DATA, 'Maps', folder, name);
    if (fs.existsSync(file)) files.push({ path: `${spawns.MAPS_DIR}/${folder}/${name}`, content: read(file), map: spawns.folderMap(folder) });
  }
  return files;
}

test('zones: los Zones.toml del repo son los que arma el panel desde los .txt', () => {
  const files = repoFiles(zones.ZONE_FILE);
  assert.ok(files.length > 0);
  const names = new Map(maps.parseToml(repoFiles(maps.MAP_FILE)).map((m) => [m.index, m.name]));
  const folders = new Map(fs.readdirSync(path.join(DATA, 'Maps')).map((f) => [spawns.folderMap(f), f]));
  const loaded = zones.parseToml(files);
  const data = {};
  for (const kind of zones.KINDS) data[kind] = zones.parseLegacy(kind, zones.buildLegacy(kind, loaded[kind]));
  const built = zones.buildToml(data, { mapName: (id) => names.get(id), folders });
  assert.deepEqual(Object.keys(built).sort(), files.map((f) => f.path).sort());
  for (const f of files) assert.equal(built[f.path], f.content, f.path);
});

test('zones: cada lista va al archivo de su mapa, con su comentario', () => {
  const data = {
    safezone: zones.parseLegacy('safezone', '6\t42\t68\t53\t98\t//Jaula\nend\n'),
    pkfree: zones.parseLegacy('pkfree', '0\t1\t2\t3\t4\nend\n'),
    npcmove: zones.parseLegacy('npcmove', '247\t0\t145\t157\t37\t227\t15\t50\t*\t*\t*\t1\t1\t//Greed Island\nend\n')
  };
  const files = zones.buildToml(data, { mapName: (id) => ({ 0: 'Lorencia', 6: 'Arena' })[id] });
  const lore = files[`${spawns.MAPS_DIR}/00-Lorencia/${zones.ZONE_FILE}`];
  const arena = files[`${spawns.MAPS_DIR}/06-Arena/${zones.ZONE_FILE}`];
  assert.match(arena, /^safe = \[\n {2}\[42, 68, 53, 98\], # Jaula\n\]$/m);
  assert.match(lore, /^pk_free = \[\n {2}\[1, 2, 3, 4\],\n\]$/m);
  assert.match(lore, /\{ npc = 247, at = \[145, 157\], to = \[37, 227, 15\], level = \[50, -1\], account = 1, pk_move = true \}, # Greed Island/);
  const back = zones.parseToml(Object.entries(files).map(([p, content]) => ({ path: p, content, map: spawns.folderMap(p.split('/')[2]) })));
  for (const kind of zones.KINDS) assert.equal(zones.buildLegacy(kind, back[kind]), zones.buildLegacy(kind, data[kind]), kind);
});

test('zones: errores con el archivo y el campo', () => {
  const file = (body) => [{ path: 'Data/Maps/00-Lorencia/Zones.toml', content: body, map: 0 }];
  assert.throws(() => zones.parseToml(file('pvp = []\n')), /sección desconocida 'pvp'/);
  assert.throws(() => zones.parseToml(file('safe = [\n  [1, 2, 3],\n]\n')), /\[x, y, tx, ty\]/);
  assert.throws(() => zones.parseToml(file('npc_move = [\n  { npc = 1, at = [1, 2], to = [0, 1, 2], vip = 1 },\n]\n')), /campo desconocido 'vip'/);
  assert.throws(() => zones.parseToml(file('npc_move = [\n  { at = [1, 2], to = [0, 1, 2] },\n]\n')), /falta 'npc'/);
  assert.throws(() => zones.parseToml(file('npc_move = [\n  { npc = 1, at = [1, 2], to = [0, 1] },\n]\n')), /'to' es una lista de 3/);
  assert.throws(() => zones.parseLegacy('safezone', '0\t1\t2\n'), /5 columnas/);
});
