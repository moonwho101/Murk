import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { World } from '../src/world.js';

const episodes = JSON.parse(readFileSync(new URL('../src/data/episodes.json', import.meta.url)));

/** A simple autopilot: fetch the chest, fight what is in the way, bring it home. */
function runBot(hero, ep, seed, maxSeconds) {
	const w = new World({ hero, episode: ep, episodeNo: 1, seed });
	const p = w.player;
	const dt = 1 / 30;
	let result = 'timeout';
	w.on((type) => {
		if (type === 'won' || type === 'lost') result = type;
	});
	for (let t = 0; t < maxSeconds / dt && w.state === 'playing'; t++) {
		const chest = w.chestById('B');
		const goal = p.carrying
			? { level: w.pads[0].level, x: w.pads[0].x, y: w.pads[0].y }
			: { level: chest.level, x: Math.round(chest.x), y: Math.round(chest.y) };
		const field = w.getField('bot', goal);
		const r = p.alive ? w.routeStep(p, field) : null;
		const input = { moveX: 0, moveY: 0, fire: false, aim: null, interact: false };
		if (r && !r.arrived) {
			input.moveX = r.vx;
			input.moveY = r.vy;
			if (r.stairs) input.interact = true;
		}
		const eng = w.nearestHostile(p, 8, true);
		if (eng) {
			input.aim = Math.atan2(eng.target.y - p.y, eng.target.x - p.x);
			input.fire = true;
		}
		w.update(dt, input);
		for (const e of w.entities) {
			assert.ok(Number.isFinite(e.x) && Number.isFinite(e.y));
		}
	}
	return { result, time: w.time, kills: w.stats.kills, deaths: w.stats.deaths, lvl: p.lvl };
}

test('autopilot can complete early episodes of every hero', () => {
	const summary = [];
	for (const hero of Object.keys(episodes)) {
		for (const idx of [0, 1, 2]) {
			const ep = episodes[hero][idx];
			const r = runBot(hero, ep, 1000 + idx, 360);
			summary.push(`${hero} #${idx + 1} gt${ep.gametype}: ${r.result} in ${r.time.toFixed(0)}s kills=${r.kills} deaths=${r.deaths}`);
			assert.notEqual(r.result, 'timeout', `${hero} episode ${idx + 1} should be winnable by the bot (${JSON.stringify(r)})`);
		}
	}
	console.log(summary.join('\n'));
});
