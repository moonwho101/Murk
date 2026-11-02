import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { Rng } from '../src/rng.js';
import { generateDungeon, distanceField, T } from '../src/dungeon.js';
import { World, spriteDir } from '../src/world.js';

const episodes = JSON.parse(readFileSync(new URL('../src/data/episodes.json', import.meta.url)));

test('dungeons are connected across levels via stairs', () => {
	for (const hero of Object.keys(episodes)) {
		for (const ep of episodes[hero].slice(0, 25)) {
			const d = generateDungeon(ep, new Rng(ep.dungeonx * 977 + ep.dungeony));
			assert.equal(d.levels.length, Math.max(2, ep.dungeonlvl));
			let start = null;
			for (let y = 0; y < d.h && !start; y++) {
				for (let x = 0; x < d.w; x++) {
					if (d.levels[0].tiles[y * d.w + x] === T.FLOOR) {
						start = [x, y];
						break;
					}
				}
			}
			const f = distanceField(d, 0, start[0], start[1]);
			for (let L = 0; L < d.levels.length; L++) {
				const tiles = d.levels[L].tiles;
				for (let i = 0; i < tiles.length; i++) {
					if (tiles[i] !== T.VOID) assert.ok(f[L][i] >= 0, `unreachable tile on level ${L}`);
				}
			}
		}
	}
});

test('no 2x2 floor blocks dominate (corridor style)', () => {
	const d = generateDungeon(episodes.fighter[0], new Rng(5));
	let blocks = 0;
	let floors = 0;
	const t = d.levels[0].tiles;
	for (let y = 1; y < d.h - 1; y++) {
		for (let x = 1; x < d.w - 1; x++) {
			if (t[y * d.w + x]) floors++;
			if (t[y * d.w + x] && t[y * d.w + x + 1] && t[(y + 1) * d.w + x] && t[(y + 1) * d.w + x + 1]) blocks++;
		}
	}
	assert.ok(floors > 15);
	assert.ok(blocks <= floors * 0.15, `${blocks} blocks vs ${floors} floors`);
});

test('spriteDir quantises to the four iso directions', () => {
	assert.equal(spriteDir(-1, 0), 1);
	assert.equal(spriteDir(0, -1), 2);
	assert.equal(spriteDir(0, 1), 3);
	assert.equal(spriteDir(1, 0), 4);
});

function runWorld(hero, ep, seconds, seed) {
	const w = new World({ hero, episode: ep, episodeNo: 1, seed });
	const rng = new Rng(seed + 1);
	const events = {};
	w.on((type) => (events[type] = (events[type] || 0) + 1));
	let moveX = 0;
	let moveY = 0;
	for (let t = 0; t < seconds * 30; t++) {
		if (t % 20 === 0) {
			const a = rng.float(0, Math.PI * 2);
			moveX = Math.cos(a);
			moveY = Math.sin(a);
		}
		w.update(1 / 30, { moveX, moveY, fire: t % 6 === 0, aim: rng.float(0, Math.PI * 2) });
		for (const e of w.entities) {
			assert.ok(Number.isFinite(e.x) && Number.isFinite(e.y), 'entity position finite');
			assert.ok(w.tile(e.level, Math.round(e.x), Math.round(e.y)) !== T.VOID, `entity ${e.type} inside a wall`);
		}
	}
	return { w, events };
}

test('simulation runs for every hero without errors', () => {
	for (const hero of Object.keys(episodes)) {
		for (const idx of [0, 2, 7, 12]) {
			const ep = episodes[hero][idx];
			const { w } = runWorld(hero, ep, 40, 100 + idx);
			assert.ok(w.player);
		}
	}
});

test('monsters wake near the player and can be killed', () => {
	const ep = { ...episodes.fighter[0], skeletons: 6, dungeonlvl: 2 };
	const w = new World({ hero: 'fighter', episode: ep, seed: 7 });
	const p = w.player;
	const mon = w.entities.find((e) => e.kind === 'monster');
	assert.ok(mon.asleep);
	w.teleportTo(p, mon.level, mon.x, mon.y);
	w.update(1 / 30, {});
	assert.ok(!mon.asleep, 'monster should wake');
	mon.hp = 1;
	w.applyDamage(mon, 5, p);
	assert.ok(!mon.alive);
	assert.ok(p.xp > 0);
});

test('chest can be taken and returned for victory', () => {
	const ep = { ...episodes.fighter[0], skeletons: 0, mouther: 0 };
	const w = new World({ hero: 'fighter', episode: ep, seed: 11 });
	const chest = w.chestById('B');
	const p = w.player;
	w.teleportTo(p, chest.level, chest.x, chest.y);
	w.update(1 / 30, {});
	assert.equal(p.carrying, 'B');
	const pad = w.pads[0];
	w.teleportTo(p, pad.level, pad.x, pad.y);
	w.update(1 / 30, {});
	assert.equal(w.state, 'won');
});

test('stairs move the player between levels', () => {
	const w = new World({ hero: 'thief', episode: episodes.thief[0], seed: 3 });
	const p = w.player;
	const s = w.dungeon.levels[0].stairs.find((s) => s.kind === 'down');
	assert.ok(s);
	w.teleportTo(p, 0, s.x, s.y);
	p.stairsArmed = true;
	assert.ok(w.useStairs(p));
	assert.equal(p.level, 1);
});
