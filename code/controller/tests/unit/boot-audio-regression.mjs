import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const services = join(here, '..', '..', 'main', 'services');

const inputs = [
  { file: 'exit.c', collection: 'exits', count: 'NUM_OF_EXITS', read: 'get_io', invertible: true },
  { file: 'motion.c', collection: 'motions', count: 'NUM_OF_MOTIONS', read: 'get_mcp_io' },
  { file: 'keypad.c', collection: 'keypads', count: 'NUM_OF_KEYPADS', read: 'get_io' },
  { file: 'fob.c', collection: 'fobs', count: 'NUM_OF_FOBS', read: 'get_mcp_io' },
];

for (const input of inputs) {
  const source = readFileSync(join(services, input.file), 'utf8');
  const firstTask = source.indexOf('xTaskCreate(');
  assert.notEqual(firstTask, -1, `${input.file}: expected service tasks`);
  const initialization = source.slice(0, firstTask);

  assert.match(
    initialization,
    new RegExp(`for \\(int i = 0; i < ${input.count}; i\\+\\+\\)[\\s\\S]*?${input.collection}\\[i\\]\\.expired = true;[\\s\\S]*?${input.collection}\\[i\\]\\.count = 0;`),
    `${input.file}: every re-arm timer must start expired with a zero count`,
  );
  const sample = input.invertible
    ? `${input.collection}\\[i\\]\\.invert \\? ${input.read}\\(${input.collection}\\[i\\]\\.pin\\) : !${input.read}\\(${input.collection}\\[i\\]\\.pin\\)`
    : `!${input.read}\\(${input.collection}\\[i\\]\\.pin\\)`;
  assert.match(
    initialization,
    new RegExp(`${input.collection}\\[i\\]\\.isPressed = ${sample};[\\s\\S]*?${input.collection}\\[i\\]\\.prevPress = ${input.collection}\\[i\\]\\.isPressed;`),
    `${input.file}: hardware state must be sampled before tasks start`,
  );
  assert.equal(
    source.includes('alert_output_signal_force('),
    false,
    `${input.file}: normal input events must honor quiet mode`,
  );

  const restore = source.search(/restore(?:Exit|Motion|Keypad|Fob)Settings\(\)/);
  assert.ok(restore >= 0 && restore < firstTask, `${input.file}: restore settings before tasks start`);
}

const lock = readFileSync(join(services, 'lock.c'), 'utf8');
const timerBody = lock.match(/void start_lock_contact_timer\(Lock \*lck, bool val\) \{([\s\S]*?)\n\}/)?.[1] ?? '';
assert.match(timerBody, /lck->count = 0;/, 'lock contact delay must restart from zero');
const lockInit = lock.match(/void lock_init\(\)\s*\{([\s\S]*?)\n\}/)?.[1] ?? '';
assert.match(
  lockInit,
  /for \(int i=0; i < NUM_OF_LOCKS; i\+\+\) \{[\s\S]*?locks\[i\]\.expired = true;[\s\S]*?locks\[i\]\.count = 0;/,
  'lock contact timers must start expired with a zero count',
);
assert.equal(
  lock.includes('alert_output_signal_force('),
  false,
  'normal lock actions must honor quiet mode',
);

const api = readFileSync(join(services, 'api.c'), 'utf8');
assert.match(
  api,
  /#define RESTART_TASK_STACK_SIZE\s+4096/,
  'restart tasks need enough stack for registered shutdown handlers',
);
assert.match(
  api,
  /xTaskCreate\(ota_reboot_task,\s*"ota_reboot",\s*RESTART_TASK_STACK_SIZE,/,
  'OTA must use the guarded restart stack size',
);

console.log('boot-audio-regression: all source contracts passed');
