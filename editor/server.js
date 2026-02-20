const express = require('express');
const fs = require('fs');
const fsp = fs.promises;
const path = require('path');
const { execFile } = require('child_process');

const app = express();

const PORT = Number(process.env.PORT || process.env.EDITOR_PORT || 8090);
const MU_ROOT = process.env.EDITOR_MU_ROOT || '/mu';
const BACKUP_DIR = process.env.EDITOR_BACKUP_DIR || path.join(MU_ROOT, '.editor-backups');
const MAX_BACKUPS = Math.max(0, Number.parseInt(process.env.EDITOR_MAX_BACKUPS || '5', 10) || 5);
const SNAPSHOT_DIR = path.join(BACKUP_DIR, '_snapshots');
const MAX_SNAPSHOTS = Math.max(0, Number.parseInt(process.env.EDITOR_MAX_SNAPSHOTS || '5', 10) || 5);

const ALLOWED_PREFIXES = [
  'Data/',
  'GameServer/DATA/',
  'GameServer/Data/'
];

function isEnabled() {
  const value = String(process.env.EDITOR_ENABLED || '0').toLowerCase();
  return value === '1' || value === 'true' || value === 'yes';
}

function execTar(args) {
  return new Promise((resolve, reject) => {
    execFile('tar', args, (err, stdout, stderr) => {
      if (err) {
        return reject(new Error(stderr || err.message));
      }
      return resolve(stdout);
    });
  });
}

function sanitizeLabel(label) {
  const raw = String(label || '').trim();
  if (!raw) return '';
  return raw.toLowerCase().replace(/[^a-z0-9_-]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 40);
}

function normalizeRelativePath(input) {
  const raw = String(input || '').trim().replace(/\\/g, '/');
  if (!raw || raw.includes('\u0000')) {
    throw new Error('Invalid path');
  }

  const hasPrefix = ALLOWED_PREFIXES.some((prefix) => raw.startsWith(prefix));
  if (!hasPrefix) {
    throw new Error('Path not allowed');
  }

  const abs = path.resolve(MU_ROOT, raw);
  const muRootAbs = path.resolve(MU_ROOT) + path.sep;
  if (!abs.startsWith(muRootAbs)) {
    throw new Error('Path escapes root');
  }

  return { rel: raw, abs };
}

async function ensureBackup(filePath, relPath) {
  if (!MAX_BACKUPS) {
    return [];
  }

  try {
    await fsp.access(filePath, fs.constants.F_OK);
  } catch {
    return [];
  }

  const dir = path.join(BACKUP_DIR, relPath);
  await fsp.mkdir(dir, { recursive: true });

  const stamp = new Date().toISOString().replace(/[:.]/g, '-');
  const backupPath = path.join(dir, `${stamp}.bak`);
  await fsp.copyFile(filePath, backupPath);

  const files = await fsp.readdir(dir);
  const withStats = await Promise.all(
    files.map(async (name) => {
      const full = path.join(dir, name);
      const stat = await fsp.stat(full);
      return { name, full, mtime: stat.mtimeMs };
    })
  );

  withStats.sort((a, b) => b.mtime - a.mtime);
  const keep = withStats.slice(0, MAX_BACKUPS);
  const remove = withStats.slice(MAX_BACKUPS);

  await Promise.all(remove.map((entry) => fsp.unlink(entry.full)));

  return keep.map((entry) => entry.name);
}

async function listBackups(relPath) {
  const dir = path.join(BACKUP_DIR, relPath);
  try {
    const files = await fsp.readdir(dir);
    const withStats = await Promise.all(
      files.map(async (name) => {
        const full = path.join(dir, name);
        const stat = await fsp.stat(full);
        return { name, mtime: stat.mtimeMs, size: stat.size };
      })
    );
    withStats.sort((a, b) => b.mtime - a.mtime);
    return withStats.slice(0, MAX_BACKUPS);
  } catch {
    return [];
  }
}

async function listSnapshots() {
  try {
    const files = await fsp.readdir(SNAPSHOT_DIR);
    const withStats = await Promise.all(
      files.map(async (name) => {
        const full = path.join(SNAPSHOT_DIR, name);
        const stat = await fsp.stat(full);
        return { name, mtime: stat.mtimeMs, size: stat.size };
      })
    );
    withStats.sort((a, b) => b.mtime - a.mtime);
    return withStats.slice(0, MAX_SNAPSHOTS);
  } catch {
    return [];
  }
}

function resolveGameServerDataRel() {
  const preferred = [
    'GameServer/DATA',
    'GameServer/Data'
  ];
  for (const rel of preferred) {
    const full = path.join(MU_ROOT, rel);
    if (fs.existsSync(full)) {
      return rel;
    }
  }
  return 'GameServer/DATA';
}

async function createSnapshot(label) {
  if (!MAX_SNAPSHOTS) {
    return { name: null, snapshots: [] };
  }

  await fsp.mkdir(SNAPSHOT_DIR, { recursive: true });
  const stamp = new Date().toISOString().replace(/[:.]/g, '-');
  const safeLabel = sanitizeLabel(label);
  const name = safeLabel ? `snapshot-${stamp}-${safeLabel}.tgz` : `snapshot-${stamp}.tgz`;
  const snapshotPath = path.join(SNAPSHOT_DIR, name);
  const gameServerDataRel = resolveGameServerDataRel();

  await execTar(['-czf', snapshotPath, '-C', MU_ROOT, 'Data', gameServerDataRel]);

  const entries = await fsp.readdir(SNAPSHOT_DIR);
  const withStats = await Promise.all(
    entries
      .filter((entry) => entry.endsWith('.tgz'))
      .map(async (entry) => {
        const full = path.join(SNAPSHOT_DIR, entry);
        const stat = await fsp.stat(full);
        return { entry, full, mtime: stat.mtimeMs };
      })
  );
  withStats.sort((a, b) => b.mtime - a.mtime);
  const toDelete = withStats.slice(MAX_SNAPSHOTS);
  await Promise.all(toDelete.map((entry) => fsp.unlink(entry.full)));

  const snapshots = await listSnapshots();
  return { name, snapshots };
}

async function restoreSnapshot(name) {
  if (!name) {
    throw new Error('Snapshot is required');
  }

  const snapshotPath = path.resolve(SNAPSHOT_DIR, name);
  const snapshotRoot = path.resolve(SNAPSHOT_DIR) + path.sep;
  if (!snapshotPath.startsWith(snapshotRoot)) {
    throw new Error('Invalid snapshot path');
  }

  await execTar(['-xzf', snapshotPath, '-C', MU_ROOT]);
  return listSnapshots();
}

app.use(express.json({ limit: '5mb' }));

app.get('/health', (req, res) => {
  res.json({
    enabled: isEnabled(),
    maxBackups: MAX_BACKUPS,
    maxSnapshots: MAX_SNAPSHOTS,
    allowedPrefixes: ALLOWED_PREFIXES
  });
});

app.use('/api', (req, res, next) => {
  if (!isEnabled()) {
    return res.status(503).json({ error: 'Editor API disabled' });
  }
  return next();
});

app.get('/api/status', (req, res) => {
  res.json({
    enabled: true,
    maxBackups: MAX_BACKUPS,
    maxSnapshots: MAX_SNAPSHOTS,
    allowedPrefixes: ALLOWED_PREFIXES
  });
});

app.get('/api/file', async (req, res) => {
  try {
    const { abs, rel } = normalizeRelativePath(req.query.path);
    const content = await fsp.readFile(abs, 'utf8');
    res.json({ path: rel, content });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.get('/api/binary', async (req, res) => {
  try {
    const { abs, rel } = normalizeRelativePath(req.query.path);
    const data = await fsp.readFile(abs);
    res.json({
      path: rel,
      base64: data.toString('base64'),
      size: data.length
    });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.get('/api/list', async (req, res) => {
  try {
    const { abs, rel } = normalizeRelativePath(req.query.path);
    const stat = await fsp.stat(abs);
    if (!stat.isDirectory()) {
      throw new Error('Not a directory');
    }
    const entries = await fsp.readdir(abs, { withFileTypes: true });
    const data = entries.map((entry) => ({
      name: entry.name,
      type: entry.isDirectory() ? 'dir' : 'file'
    }));
    res.json({ path: rel, entries: data });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.post('/api/file', async (req, res) => {
  try {
    const { path: relPath, content } = req.body || {};
    const { abs, rel } = normalizeRelativePath(relPath);
    await ensureBackup(abs, rel);
    await fsp.writeFile(abs, String(content ?? ''), 'utf8');
    const backups = await listBackups(rel);
    res.json({ ok: true, path: rel, backups });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.get('/api/backups', async (req, res) => {
  try {
    const { rel } = normalizeRelativePath(req.query.path);
    const backups = await listBackups(rel);
    res.json({ path: rel, backups });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.post('/api/restore', async (req, res) => {
  try {
    const { path: relPath, backup } = req.body || {};
    const { abs, rel } = normalizeRelativePath(relPath);
    if (!backup) {
      throw new Error('Backup is required');
    }
    const backupPath = path.join(BACKUP_DIR, rel, backup);
    const backupAbs = path.resolve(backupPath);
    const backupRootAbs = path.resolve(BACKUP_DIR) + path.sep;
    if (!backupAbs.startsWith(backupRootAbs)) {
      throw new Error('Invalid backup path');
    }
    const content = await fsp.readFile(backupAbs, 'utf8');
    await ensureBackup(abs, rel);
    await fsp.writeFile(abs, content, 'utf8');
    const backups = await listBackups(rel);
    res.json({ ok: true, path: rel, backups });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.get('/api/backup/list', async (req, res) => {
  try {
    const snapshots = await listSnapshots();
    res.json({ snapshots });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.post('/api/backup/create', async (req, res) => {
  try {
    const { label } = req.body || {};
    const result = await createSnapshot(label);
    res.json({ ok: true, name: result.name, snapshots: result.snapshots });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.post('/api/backup/restore', async (req, res) => {
  try {
    const { name } = req.body || {};
    const snapshots = await restoreSnapshot(name);
    res.json({ ok: true, snapshots });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.post('/api/reload', async (req, res) => {
  try {
    const target = String(req.body?.target || '').toLowerCase();
    const allowed = [
      'shop',
      'monster',
      'move',
      'gate',
      'quest',
      'util',
      'item',
      'skill',
      'event',
      'eventitembag',
      'chaosmix',
      'command',
      'custom',
      'character',
      'map',
      'hack',
      'all'
    ];
    if (!allowed.includes(target)) {
      throw new Error('Unsupported reload target');
    }
    const { abs, rel } = normalizeRelativePath('Data/EditorReload.flag');
    let existing = '';
    try {
      existing = await fsp.readFile(abs, 'utf8');
    } catch {
      existing = '';
    }
    const tokens = existing
      .split(/[,\s]+/g)
      .map((value) => value.trim().toLowerCase())
      .filter(Boolean);
    if (!tokens.includes(target)) {
      tokens.push(target);
    }
    const content = tokens.join(',') + '\n';
    await fsp.writeFile(abs, content, 'utf8');
    res.json({ ok: true, target, path: rel, queued: tokens });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

app.listen(PORT, () => {
  console.log(`[mu-editor] listening on :${PORT} (enabled=${isEnabled()})`);
});
