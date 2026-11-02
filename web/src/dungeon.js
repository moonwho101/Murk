import { Rng } from './rng.js';
import { THEMES } from './config.js';

export const T = { VOID: 0, FLOOR: 1, DOWN: 2, UP: 3 };

export const DIRS4 = [
	[-1, 0], // sprite dir 1: grid x-1 (screen up-left)
	[0, -1], // 2: grid y-1 (up-right)
	[0, 1], // 3: grid y+1 (down-left)
	[1, 0], // 4: grid x+1 (down-right)
];

/**
 * Port of Murk's "drunken walker" cave carver. Walkers wander from the middle of
 * the map and refuse to carve 2x2 floor blocks, producing winding one-tile
 * corridors and little chambers like the original dungeons.
 */
function carve(w, h, dx, dy, rng, jumps, steps) {
	const tiles = new Uint8Array(w * h);
	const idx = (x, y) => y * w + x;
	const sx = Math.floor(dx / 2);
	const sy = Math.floor(dy / 2);
	tiles[idx(sx, sy)] = T.FLOOR;
	const floors = [[sx, sy]];

	const makesBlock = (nx, ny) => {
		for (const [ox, oy] of [[-1, -1], [1, -1], [-1, 1], [1, 1]]) {
			if (
				tiles[idx(nx + ox, ny)] === T.FLOOR &&
				tiles[idx(nx, ny + oy)] === T.FLOOR &&
				tiles[idx(nx + ox, ny + oy)] === T.FLOOR
			) {
				return true;
			}
		}
		return false;
	};

	for (let j = 0; j < jumps; j++) {
		let [x, y] = j < 2 ? [sx, sy] : rng.pick(floors);
		for (let i = 0; i < steps; i++) {
			const [mx, my] = DIRS4[rng.int(4)];
			const nx = x + mx;
			const ny = y + my;
			if (nx <= 0 || nx >= dx || ny <= 0 || ny >= dy) continue;
			if (!rng.chance(1 / 30) && makesBlock(nx, ny)) continue;
			if (tiles[idx(nx, ny)] !== T.FLOOR) {
				tiles[idx(nx, ny)] = T.FLOOR;
				floors.push([nx, ny]);
			}
			x = nx;
			y = ny;
		}
	}
	return tiles;
}

function countFloors(tiles) {
	let n = 0;
	for (let i = 0; i < tiles.length; i++) if (tiles[i]) n++;
	return n;
}

/**
 * @param {{dungeonx:number,dungeony:number,dungeonlvl:number}} def
 */
export function generateDungeon(def, rng = new Rng()) {
	const dx = Math.max(8, def.dungeonx);
	const dy = Math.max(8, def.dungeony);
	const count = Math.max(2, def.dungeonlvl);
	const w = dx + 2;
	const h = dy + 2;
	const area = dx * dy;
	const jumps = Math.min(11, Math.max(5, Math.round(area / 42)));
	const steps = 80;
	const minFloors = Math.floor(area * 0.2);
	const stairsPerLink = area >= 220 ? 3 : 2;

	const idx = (x, y) => y * w + x;
	const levels = [];
	const themes = pickThemes(count, rng);

	for (let L = 0; L < count; L++) {
		let level = null;
		for (let attempt = 0; attempt < 60 && !level; attempt++) {
			const tiles = carve(w, h, dx, dy, rng, jumps, steps);
			if (countFloors(tiles) < minFloors) continue;
			let stairs = [];
			if (L > 0) {
				const prev = levels[L - 1];
				const cand = [];
				for (let y = 1; y <= dy; y++) {
					for (let x = 1; x <= dx; x++) {
						if (tiles[idx(x, y)] === T.FLOOR && prev.tiles[idx(x, y)] === T.FLOOR) cand.push([x, y]);
					}
				}
				rng.shuffle(cand);
				for (const [x, y] of cand) {
					if (stairs.length >= stairsPerLink) break;
					const near = (arr) => arr.some(([sx, sy]) => Math.abs(sx - x) + Math.abs(sy - y) < 4);
					const prevNear = prev.stairs.some((s) => Math.abs(s.x - x) + Math.abs(s.y - y) < 3);
					if (near(stairs) || prevNear) continue;
					stairs.push([x, y]);
				}
				if (stairs.length < Math.min(2, stairsPerLink)) continue;
			}
			level = { index: L, tiles, stairs: [], theme: themes[L] };
			for (const [x, y] of stairs) {
				const prev = levels[L - 1];
				tiles[idx(x, y)] = T.UP;
				prev.tiles[idx(x, y)] = T.DOWN;
				level.stairs.push({ x, y, kind: 'up' });
				prev.stairs.push({ x, y, kind: 'down' });
			}
		}
		if (!level) throw new Error('dungeon generation failed');
		levels.push(level);
	}
	return { w, h, dx, dy, levels };
}

function pickThemes(count, rng) {
	const pool = rng.shuffle(THEMES.slice(1));
	const out = ['murk'];
	for (let i = 1; i < count; i++) out.push(pool[(i - 1) % pool.length]);
	// the original mossy dungeon always opens the first level
	return out;
}

export function tileAt(dungeon, L, x, y) {
	if (x < 0 || y < 0 || x >= dungeon.w || y >= dungeon.h) return T.VOID;
	return dungeon.levels[L].tiles[y * dungeon.w + x];
}

export function isWalkable(dungeon, L, x, y) {
	return tileAt(dungeon, L, x, y) !== T.VOID;
}

/** Stairs partner: the same x,y one level up/down. */
export function stairsTarget(dungeon, L, x, y) {
	const t = tileAt(dungeon, L, x, y);
	if (t === T.DOWN && L + 1 < dungeon.levels.length) return { level: L + 1, x, y };
	if (t === T.UP && L > 0) return { level: L - 1, x, y };
	return null;
}

/**
 * Multi-level BFS distance field to (level,x,y), walking through stairs.
 * Returns an array of Int16Array per level (-1 = unreachable).
 */
export function distanceField(dungeon, level, x, y, useStairs = true) {
	const { w, h } = dungeon;
	const n = w * h;
	const field = dungeon.levels.map(() => new Int16Array(n).fill(-1));
	const qL = [];
	const qI = [];
	field[level][y * w + x] = 0;
	qL.push(level);
	qI.push(y * w + x);
	for (let head = 0; head < qL.length; head++) {
		const L = qL[head];
		const i = qI[head];
		const d = field[L][i];
		const cx = i % w;
		const cy = (i / w) | 0;
		const tiles = dungeon.levels[L].tiles;
		for (const [ox, oy] of DIRS4) {
			const nx = cx + ox;
			const ny = cy + oy;
			if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
			const ni = ny * w + nx;
			if (tiles[ni] === T.VOID || field[L][ni] !== -1) continue;
			field[L][ni] = d + 1;
			qL.push(L);
			qI.push(ni);
		}
		if (!useStairs) continue;
		const t = tiles[i];
		let nl = -1;
		if (t === T.DOWN && L + 1 < dungeon.levels.length) nl = L + 1;
		else if (t === T.UP && L > 0) nl = L - 1;
		if (nl >= 0 && field[nl][i] === -1) {
			field[nl][i] = d + 1;
			qL.push(nl);
			qI.push(i);
		}
	}
	return field;
}

/** Tiles reachable from (x,y) within `radius` steps on a single level (for vision). */
export function floodVisible(dungeon, L, x, y, radius) {
	const { w, h } = dungeon;
	const tiles = dungeon.levels[L].tiles;
	const seen = new Int8Array(w * h).fill(-1);
	const q = [y * w + x];
	seen[q[0]] = 0;
	for (let head = 0; head < q.length; head++) {
		const i = q[head];
		const d = seen[i];
		if (d >= radius) continue;
		const cx = i % w;
		const cy = (i / w) | 0;
		for (const [ox, oy] of DIRS4) {
			const nx = cx + ox;
			const ny = cy + oy;
			if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
			const ni = ny * w + nx;
			if (tiles[ni] === T.VOID || seen[ni] !== -1) continue;
			seen[ni] = d + 1;
			q.push(ni);
		}
	}
	return seen;
}

/** Walkable straight-line check between two points (tile sampling). */
export function lineClear(dungeon, L, x0, y0, x1, y1) {
	const dist = Math.hypot(x1 - x0, y1 - y0);
	const steps = Math.ceil(dist / 0.25);
	for (let i = 1; i < steps; i++) {
		const t = i / steps;
		const x = Math.round(x0 + (x1 - x0) * t);
		const y = Math.round(y0 + (y1 - y0) * t);
		if (tileAt(dungeon, L, x, y) === T.VOID) return false;
	}
	return true;
}
