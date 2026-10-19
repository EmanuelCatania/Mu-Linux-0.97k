const test = require('node:test');
const assert = require('node:assert/strict');
const { parseGameMaster, staffFromLevel, buildStaffIndex, lookupStaff } = require('../lib/game-master');

test('staff level maps GM and admin separately', () => {
  assert.equal(staffFromLevel(0), null);
  assert.deepEqual(staffFromLevel(1), { role: 'gm', label: 'GM' });
  assert.deepEqual(staffFromLevel(2), { role: 'admin', label: 'Admin' });
  assert.deepEqual(staffFromLevel(3), { role: 'admin', label: 'Admin' });
});

test('GameMaster.txt stops at end and keeps the higher role for a name', () => {
  const content = [
    '//Account Name Level',
    '"acc"\t"Hero"\t1',
    '"other" "Hero" 2',
    '"acc" "Boss" 3',
    'end',
    '"ignored" "Ghost" 1'
  ].join('\n');
  const index = buildStaffIndex(parseGameMaster(content));
  assert.equal(index.names.length, 2);
  assert.equal(lookupStaff(index, 'hero').role, 'admin');
  assert.equal(lookupStaff(index, 'hero').level, 2);
  assert.equal(lookupStaff(index, 'Boss').role, 'admin');
  assert.equal(lookupStaff(index, 'Ghost'), null);
});
