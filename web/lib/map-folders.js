// Editor por carpeta de mapa: lee, valida, guarda y borra el TOML de un tipo
// para un mapa de Data/Maps/NN-Nombre. Al guardar se valida contra los archivos
// del mismo tipo de los otros mapas (los números de gate no se pueden repetir,
// por ejemplo) y se escribe sólo ese archivo.
//
// El acceso a los archivos se inyecta (`io`), para que el panel use el editor y
// los tests usen el disco:
//   io.list(dir)          -> [{ name, type: 'file'|'dir' }] o null si no existe
//   io.read(path)         -> texto
//   io.write(path, text)  -> { unchanged?, backups? }
//   io.remove(path)       -> { backups? }
const spawns = require('./spawns');
const maps = require('./maps');
const drops = require('./drops');
const gates = require('./gates');
const zones = require('./zones');

// `reload` es lo que hay que pedirle al GameServer para que lo aplique.
const KINDS = {
  map: { file: maps.MAP_FILE, lib: maps, Err: maps.MapError, reload: 'mapmanager', deletable: true },
  spawns: { file: 'Spawns.toml', lib: spawns, Err: spawns.SpawnError, reload: 'spawns', deletable: false },
  drops: { file: drops.DROP_FILE, lib: drops, Err: drops.DropError, reload: 'item', deletable: true },
  gates: { file: gates.GATE_FILE, lib: gates, Err: gates.GateError, reload: 'move', deletable: true },
  zones: { file: zones.ZONE_FILE, lib: zones, Err: zones.ZoneError, reload: 'custom', deletable: true }
};

class MapFolderError extends Error {
  constructor(message, status = 400) {
    super(message);
    this.status = status;
  }
}

// 400 para los errores de validación, el status propio para el resto.
function errorStatus(err) {
  if (err instanceof MapFolderError) return err.status;
  if (Object.values(KINDS).some((d) => err instanceof d.Err)) return 400;
  return err.status || 502;
}

function kindDef(kind) {
  const def = KINDS[kind];
  if (!def) throw new MapFolderError('Tipo de archivo desconocido.');
  return def;
}

const filePath = (folder, kind) => `${spawns.MAPS_DIR}/${folder}/${kindDef(kind).file}`;
const shortName = (folder) => folder.replace(/^\d+-/, '');

function create(io) {
  // Las carpetas de mapa con los tipos que tiene cada una, con el mismo
  // criterio que el GameServer (número de la carpeta, una carpeta por mapa).
  async function listFolders() {
    const folders = [];
    for (const entry of (await io.list(spawns.MAPS_DIR)) || []) {
      const map = spawns.folderMap(entry.name);
      if (entry.type !== 'dir' || map < 0) continue;
      const other = folders.find((f) => f.map === map);
      if (other) throw new MapFolderError(`Dos carpetas para el mapa ${map}: '${other.folder}' y '${entry.name}'`);
      const inside = (await io.list(`${spawns.MAPS_DIR}/${entry.name}`)) || [];
      const kinds = Object.keys(KINDS).filter((k) => inside.some((f) => f.type === 'file' && f.name === KINDS[k].file));
      folders.push({ folder: entry.name, map, kinds });
    }
    return folders.sort((a, b) => a.map - b.map);
  }

  async function hasGlobalDrops() {
    const dir = drops.GLOBAL_FILE.slice(0, drops.GLOBAL_FILE.lastIndexOf('/'));
    return ((await io.list(dir)) || []).some((f) => f.type === 'file' && `${dir}/${f.name}` === drops.GLOBAL_FILE);
  }

  // El GameServer usa los TOML de un tipo si hay al menos uno (para los drops
  // cuenta también Data/Items/Drops.toml). Si no hay ninguno lee el .txt.
  async function usesToml(folders, kind) {
    if (folders.some((f) => f.kinds.includes(kind))) return true;
    return kind === 'drops' && hasGlobalDrops();
  }

  function findFolder(folders, name) {
    const f = folders.find((x) => x.folder === name);
    if (!f) throw new MapFolderError('Carpeta de mapa desconocida.', 404);
    return f;
  }

  // Parsea todos los archivos del tipo con `content` en lugar del de `target`
  // (o sin él, si `content` es null). Tira el error del módulo si no valida.
  async function validate(folders, kind, target, content) {
    const files = [];
    for (const f of folders) {
      const path = filePath(f.folder, kind);
      if (f.folder === target.folder) {
        if (content !== null) files.push({ path, map: f.map, content });
      } else if (f.kinds.includes(kind)) {
        files.push({ path, map: f.map, content: await io.read(path) });
      }
    }
    if (kind === 'drops' && await hasGlobalDrops()) {
      files.push({ path: drops.GLOBAL_FILE, map: -1, content: await io.read(drops.GLOBAL_FILE) });
    }
    kindDef(kind).lib.parseToml(files);
  }

  // Para un archivo nuevo se propone la explicación de los campos que traen los
  // de los otros mapas (las líneas de comentario del principio, sin la primera,
  // que es el nombre del mapa).
  async function template(folders, kind, target) {
    let help = '';
    const sample = folders.find((f) => f.kinds.includes(kind));
    if (sample) {
      const head = [];
      for (const line of (await io.read(filePath(sample.folder, kind))).replace(/\r\n/g, '\n').split('\n').slice(1)) {
        if (!line.startsWith('#') && line.trim() !== '') break;
        head.push(line);
      }
      help = head.join('\n').replace(/\n+$/, '');
    }
    const body = kind === 'map' ? `name = "${shortName(target.folder)}"\n` : '';
    return `# ${shortName(target.folder)} (mapa ${target.map})\n${help ? help + '\n' : ''}\n${body}`;
  }

  return {
    async summary() {
      const folders = await listFolders();
      const kinds = {};
      for (const [k, d] of Object.entries(KINDS)) {
        kinds[k] = { file: d.file, reload: d.reload, deletable: d.deletable, toml: await usesToml(folders, k) };
      }
      return { folders, kinds };
    },

    async read(folder, kind) {
      kindDef(kind);
      const folders = await listFolders();
      const target = findFolder(folders, folder);
      const path = filePath(target.folder, kind);
      const exists = target.kinds.includes(kind);
      return { path, exists, content: exists ? await io.read(path) : await template(folders, kind, target) };
    },

    // Con `check` sólo valida.
    async save(folder, kind, content, check = false) {
      const def = kindDef(kind);
      const text = String(content ?? '');
      const folders = await listFolders();
      const target = findFolder(folders, folder);
      // Crear el primer TOML de un tipo haría que el GameServer deje de leer el
      // .txt de todos los mapas y use sólo éste.
      if (!target.kinds.includes(kind) && !(await usesToml(folders, kind))) {
        throw new MapFolderError('Este tipo todavía está en el formato de un solo archivo. Pasalo a archivos por mapa desde su editor antes de crear uno.', 409);
      }
      await validate(folders, kind, target, text);
      if (check) return { ok: true, valid: true };
      const path = filePath(target.folder, kind);
      const data = (await io.write(path, text)) || {};
      return { ok: true, path, unchanged: !!data.unchanged, backups: data.backups || [], reload: def.reload };
    },

    async remove(folder, kind) {
      const def = kindDef(kind);
      if (!def.deletable) throw new MapFolderError('Este archivo no se puede borrar desde el panel.');
      const folders = await listFolders();
      const target = findFolder(folders, folder);
      if (!target.kinds.includes(kind)) throw new MapFolderError('El mapa no tiene ese archivo.', 404);
      // Sin ningún TOML del tipo el GameServer volvería al .txt, que puede no estar.
      if (!(await usesToml(folders.filter((f) => f !== target), kind))) {
        throw new MapFolderError('Es el último archivo de este tipo: sin él el GameServer volvería al formato viejo. Dejalo vacío en lugar de borrarlo.', 409);
      }
      await validate(folders, kind, target, null);
      const path = filePath(target.folder, kind);
      const data = (await io.remove(path)) || {};
      return { ok: true, path, backups: data.backups || [], reload: def.reload };
    }
  };
}

module.exports = { KINDS, MapFolderError, errorStatus, create };
