'use strict';

const test = require('node:test');
const assert = require('node:assert');
const crypto = require('crypto');
const accountPassword = require('../lib/account-password');

test('bcrypt: lo que genera make() se verifica y no pide migrar', async () => {
  const stored = await accountPassword.make('secreto1', true);
  assert.strictEqual(stored.length, accountPassword.HASH_LENGTH);
  assert.ok(accountPassword.isHash(stored));
  assert.deepStrictEqual(await accountPassword.verify(stored, 'secreto1'), { ok: true, legacy: false });
  assert.deepStrictEqual(await accountPassword.verify(stored, 'secreto2'), { ok: false, legacy: false });
});

test('hash generado por el JoinServer (crypt_blowfish, $2b$)', async () => {
  // Vector de OpenBSD/crypt_blowfish: "U*U" con sal fija.
  const stored = Buffer.from('$2a$05$CCCCCCCCCCCCCCCCCCCCC.E5YPO9kmyuRGyh0XouQYb4YMJKvyOeW', 'latin1');
  assert.deepStrictEqual(await accountPassword.verify(stored, 'U*U'), { ok: true, legacy: false });
});

test('MD5 binario viejo: entra y pide migrar', async () => {
  const stored = crypto.createHash('md5').update('clave99').digest();
  assert.deepStrictEqual(await accountPassword.verify(stored, 'clave99'), { ok: true, legacy: true });
  assert.deepStrictEqual(await accountPassword.verify(stored, 'clave98'), { ok: false, legacy: false });
});

test('texto plano viejo: entra y pide migrar', async () => {
  assert.deepStrictEqual(await accountPassword.verify(Buffer.from('abc123'), 'abc123'), { ok: true, legacy: true });
});

test('contraseña vacía nunca entra', async () => {
  assert.deepStrictEqual(await accountPassword.verify(Buffer.alloc(0), ''), { ok: false, legacy: false });
});

test('sin la columna ancha se sigue guardando MD5', async () => {
  const stored = await accountPassword.make('clave99', false);
  assert.ok(stored.equals(crypto.createHash('md5').update('clave99').digest()));
});
