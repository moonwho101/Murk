import { Rng } from './rng.js';
import {
	HEROES, MONSTERS, MONSTER_ORDER, MISSILES, TUNING, xpForLevel, monsterSpeedFromEpisode,
} from './config.js';
import {
	generateDungeon, T, tileAt, distanceField, floodVisible, lineClear, stairsTarget, DIRS4,
} from './dungeon.js';

let nextId = 1;

/** Map a movement/aim vector in grid space to Murk's four sprite directions (1..4). */
export function spriteDir(dx, dy) {
	if (Math.abs(dx) >= Math.abs(dy)) return dx < 0 ? 1 : 4;
	return dy < 0 ? 2 : 3;
}

const ITEM_TYPES = { e: 'elixir', a: 'armour', s: 'scroll', r: 'ring' };

export class World {
	/**
	 * @param {object} opts
	 * @param {string} opts.hero      fighter | thief | wizard | cleric
	 * @param {object} opts.episode   one record from episodes.json
	 * @param {number} opts.episodeNo 1-based episode counter
	 * @param {object} [opts.carry]   hero progression carried between episodes
	 */
	constructor({ hero, episode, episodeNo = 1, carry = null, seed }) {
		this.rng = new Rng(seed ?? (Date.now() >>> 0));
		this.def = episode;
		this.episodeNo = episodeNo;
		this.heroId = hero;
		this.listeners = [];
		this.time = 0;
		this.state = 'playing';
		this.stats = { kills: 0, deaths: 0, items: 0, shots: 0, damageTaken: 0 };
		this.entities = [];
		this.missiles = [];
		this.items = [];
		this.pits = [];
		this.chests = [];
		this.pads = [];
		this.fields = new Map();
		this.alarm = false;
		this.messageLog = [];

		this.dungeon = generateDungeon(episode, this.rng);
		const { w, h, levels } = this.dungeon;
		this.levelCount = levels.length;
		this.items = levels.map(() => new Map());
		this.explored = levels.map(() => new Uint8Array(w * h));
		this.visible = new Int8Array(w * h).fill(-1);
		this.visTile = { level: -1, x: -1, y: -1 };

		this.occupied = levels.map(() => new Set());
		this.spawnHeroes(carry);
		this.spawnMonsters();
		this.spawnChests();
		this.scatterItems();
		this.placePits();
		this.updateVision(true);
	}

	// ------------------------------------------------------------------ events
	on(fn) {
		this.listeners.push(fn);
		return () => (this.listeners = this.listeners.filter((l) => l !== fn));
	}
	emit(type, data = {}) {
		for (const l of this.listeners) l(type, data);
	}
	say(text, duration = 4, kind = 'info') {
		this.emit('message', { text, duration, kind });
	}

	// ---------------------------------------------------------- level geometry
	idx(x, y) {
		return y * this.dungeon.w + x;
	}
	tile(L, x, y) {
		return tileAt(this.dungeon, L, x, y);
	}
	floorTiles(L, filter) {
		const out = [];
		const { w, h } = this.dungeon;
		for (let y = 1; y < h - 1; y++) {
			for (let x = 1; x < w - 1; x++) {
				if (this.tile(L, x, y) === T.FLOOR && !this.occupied[L].has(y * w + x)) {
					if (!filter || filter(x, y)) out.push([x, y]);
				}
			}
		}
		return out;
	}
	takeFloor(L, filter) {
		let tiles = this.floorTiles(L, filter);
		if (!tiles.length) tiles = this.floorTiles(L);
		if (!tiles.length) return null;
		const [x, y] = this.rng.pick(tiles);
		this.occupied[L].add(this.idx(x, y));
		return { x, y };
	}

	// ------------------------------------------------------------------- spawn
	makeHero(heroKey, team, level, x, y, carry) {
		const base = HEROES[heroKey];
		const e = {
			id: nextId++, kind: 'hero', type: heroKey, base, team, level, x, y, vx: 0, vy: 0,
			dir: 3, aim: Math.PI / 2, hp: base.hp, maxHp: base.hp, armour: base.armour,
			speed: base.speed, radius: 0.27, alive: true, asleep: false, respawn: 0,
			cd: 0, shots: 0, maxShots: base.maxShots, dmg: [...base.dmg], fireCd: base.fireCd,
			hitT: 0, protT: 0, slowT: 0, flashT: 0, rings: 0, xp: 0, lvl: 1, nextXp: xpForLevel(1),
			animT: 0, moving: false, stairT: 0, stairsArmed: false, carrying: null, deathT: 0,
			home: { level, x, y }, attrib: 0, kills: 0, noChestT: 0, isPlayer: false, wander: null, aiT: 0,
		};
		if (carry) Object.assign(e, carry, { id: e.id });
		return e;
	}

	spawnHeroes(carry) {
		const L0 = 0;
		const start = this.takeFloor(L0);
		const player = this.makeHero(this.heroId, 1, L0, start.x, start.y, null);
		player.isPlayer = true;
		player.rings = 1;
		if (carry) {
			player.maxHp = carry.maxHp;
			player.hp = Math.max(carry.hp, Math.ceil(carry.maxHp * 0.6));
			player.armour = carry.armour;
			player.rings = Math.max(1, carry.rings);
			player.xp = carry.xp;
			player.lvl = carry.lvl;
			player.nextXp = carry.nextXp;
			player.levelXp = carry.levelXp || 0;
			player.dmg = [...carry.dmg];
			player.maxShots = carry.maxShots;
			player.fireCd = carry.fireCd;
			player.attrib = carry.attrib;
		}
		this.player = player;
		this.entities.push(player);
		this.pads.push({ team: 1, level: L0, x: start.x, y: start.y, color: player.base.pad });

		const d = this.def;
		const lastL = this.levelCount - 1;
		const groups = [
			['fighter', d.fighter, d.fteam, d.fspeed], ['cleric', d.cleric, d.cteam, d.cspeed],
			['thief', d.thief, d.tteam, d.tspeed], ['wizard', d.wizard, d.wteam, d.wspeed],
		];
		let enemyPad = null;
		for (const [key, count, team] of groups) {
			for (let i = 0; i < (count || 0); i++) {
				const t = team === 2 ? 2 : 1;
				let L = L0;
				let pos;
				if (t === 2) {
					L = lastL;
					if (!enemyPad) {
						enemyPad = this.takeFloor(L);
						this.pads.push({ team: 2, level: L, x: enemyPad.x, y: enemyPad.y, color: '#ff4a4a' });
					}
					pos = { x: enemyPad.x, y: enemyPad.y };
				} else {
					pos = this.takeFloor(L, (x, y) => Math.abs(x - start.x) + Math.abs(y - start.y) < 6) || start;
				}
				const e = this.makeHero(key, t, L, pos.x, pos.y, null);
				e.ai = true;
				e.hp = e.maxHp = e.base.hp + this.episodeNo * 2;
				e.speed *= 0.9;
				e.asleep = t === 2;
				if (t === 1) e.home = { level: L, x: start.x, y: start.y };
				else e.home = { level: L, x: pos.x, y: pos.y };
				this.entities.push(e);
			}
		}
	}

	spawnMonsters() {
		const d = this.def;
		const list = [
			['beholder', d.beholders, d.bspeed], ['deathorb', d.deathorb, d.dspeed],
			['skeleton', d.skeletons, d.sspeed], ['mouther', d.mouther, d.mspeed],
			['ogre', d.ogre, d.ospeed],
		];
		const start = this.pads[0];
		for (const [key, count, epSpeed] of list) {
			const base = MONSTERS[key];
			for (let i = 0; i < (count || 0); i++) {
				const L = this.levelCount > 1 ? this.rng.range(1, this.levelCount - 1) : 0;
				const pos = this.takeFloor(L, (x, y) => !(L === 0 && Math.abs(x - start.x) + Math.abs(y - start.y) < 8));
				if (!pos) continue;
				const scale = 1 + (this.episodeNo - 1) * 0.015;
				const e = {
					id: nextId++, kind: 'monster', type: key, base, team: 0, level: L, x: pos.x, y: pos.y,
					vx: 0, vy: 0, dir: this.rng.range(1, 4), aim: 0, hp: Math.round(base.hp * scale),
					maxHp: Math.round(base.hp * scale), armour: 0, speed: monsterSpeedFromEpisode(base.baseSpeed, epSpeed),
					radius: base.radius, alive: true, asleep: true, respawn: 0, cd: this.rng.float(0.5, base.shotCd),
					shots: 0, maxShots: base.maxShots, dmg: [...base.dmg], hitT: 0, protT: 0, slowT: 0, flashT: 0,
					animT: this.rng.float(0, 5), moving: false, deathT: 0, home: { level: L, x: pos.x, y: pos.y },
					wander: null, aiT: this.rng.float(0, 0.5), bob: this.rng.float(0, 6), stairT: 0,
				};
				this.entities.push(e);
			}
		}
	}

	spawnChests() {
		const gt = this.def.gametype;
		const last = this.levelCount - 1;
		const home = this.pads[0];
		// Blue chest (team 1's goal)
		const L = this.levelCount > 1 ? (this.rng.chance(0.65) ? last : this.rng.range(1, last)) : 0;
		const posB = this.takeFloor(L, (x, y) => !(L === 0 && Math.abs(x - home.x) + Math.abs(y - home.y) < 6));
		this.chests.push({
			id: 'B', color: 'blue', team: 2, level: L, x: posB.x, y: posB.y, carrier: null,
			home: { level: L, x: posB.x, y: posB.y }, noPickup: 0, bob: 0,
		});
		if (gt === 2) {
			const posA = { x: home.x, y: home.y };
			this.chests.push({
				id: 'A', color: 'red', team: 1, level: 0, x: posA.x, y: posA.y, carrier: null,
				home: { level: 0, x: posA.x, y: posA.y }, noPickup: 0, bob: 0,
			});
		}
	}

	scatterItems() {
		const lc = this.levelCount;
		let total = 16 + lc * 2;
		const maxByTiles = Math.floor((this.dungeon.dx * this.dungeon.dy * lc) * 0.09);
		total = Math.min(total, Math.max(10, maxByTiles));
		for (let i = 0; i < total; i++) {
			const L = this.rng.int(lc);
			const pos = this.takeFloor(L);
			if (!pos) continue;
			const r = this.rng.int(5);
			const type = r === 0 ? 'armour' : r === 2 ? 'scroll' : 'elixir';
			this.items[L].set(this.idx(pos.x, pos.y), { type, bob: this.rng.float(0, 6) });
		}
		for (let i = 0; i < 2; i++) {
			const L = this.rng.int(lc);
			const pos = this.takeFloor(L);
			if (pos) this.items[L].set(this.idx(pos.x, pos.y), { type: 'ring', bob: this.rng.float(0, 6) });
		}
	}

	placePits() {
		if (!this.def.pits) return;
		const count = Math.min(this.def.maxpit || 6, this.levelCount * 2 + 2);
		const { w } = this.dungeon;
		for (let i = 0; i < count; i++) {
			const L = this.rng.int(Math.max(1, this.levelCount - 1));
			if (L >= this.levelCount - 1) continue;
			const start = this.pads[0];
			const cand = this.floorTiles(L, (x, y) => this.tile(L + 1, x, y) === T.FLOOR &&
				!(L === 0 && Math.abs(x - start.x) + Math.abs(y - start.y) < 5));
			if (!cand.length) continue;
			const [x, y] = this.rng.pick(cand);
			this.occupied[L].add(y * w + x);
			this.pits.push({ level: L, x, y, t: this.rng.float(0, 10), period: this.rng.float(8, 12) });
		}
	}

	// ------------------------------------------------------------ pathfinding
	getField(key, target, useStairs = true, minAge = 0.3) {
		let f = this.fields.get(key);
		const same = f && f.tl === target.level && f.tx === target.x && f.ty === target.y;
		if (f && ((same && this.time - f.t < 2.5) || (!same && this.time - f.t < minAge))) return f.field;
		const field = distanceField(this.dungeon, target.level, target.x, target.y, useStairs);
		f = { t: this.time, tl: target.level, tx: target.x, ty: target.y, field };
		this.fields.set(key, f);
		return field;
	}

	tileOf(e) {
		return { x: Math.round(e.x), y: Math.round(e.y) };
	}

	/** Steering vector (unit) toward the next tile on the route described by `field`, plus stairs flag. */
	routeStep(e, field) {
		const { w } = this.dungeon;
		const cx = Math.round(e.x);
		const cy = Math.round(e.y);
		const d = field[e.level][cy * w + cx];
		if (d < 0) return null;
		if (d === 0) return { vx: 0, vy: 0, arrived: true };
		let best = null;
		let bd = d;
		for (const [ox, oy] of DIRS4) {
			const nx = cx + ox;
			const ny = cy + oy;
			const nd = field[e.level][ny * w + nx];
			if (nd >= 0 && nd < bd) {
				bd = nd;
				best = [ox, oy];
			}
		}
		if (best) {
			// centre on the lane, then advance
			const lx = best[1] !== 0 ? cx - e.x : 0;
			const ly = best[0] !== 0 ? cy - e.y : 0;
			let vx = best[0] + lx * 3.2;
			let vy = best[1] + ly * 3.2;
			const m = Math.hypot(vx, vy) || 1;
			vx /= m;
			vy /= m;
			return { vx, vy, arrived: false };
		}
		// stairs hop
		const tt = tileAt(this.dungeon, e.level, cx, cy);
		const nl = tt === T.DOWN ? e.level + 1 : tt === T.UP ? e.level - 1 : -1;
		if (nl >= 0 && field[nl][cy * w + cx] >= 0 && field[nl][cy * w + cx] < d) {
			return { vx: cx - e.x, vy: cy - e.y, stairs: true, arrived: false };
		}
		return null;
	}

	// ---------------------------------------------------------------- movement
	canStand(L, x, y, r) {
		const d = this.dungeon;
		return (
			tileAt(d, L, Math.round(x - r), Math.round(y - r)) !== T.VOID &&
			tileAt(d, L, Math.round(x + r), Math.round(y - r)) !== T.VOID &&
			tileAt(d, L, Math.round(x - r), Math.round(y + r)) !== T.VOID &&
			tileAt(d, L, Math.round(x + r), Math.round(y + r)) !== T.VOID
		);
	}

	moveEntity(e, vx, vy, dt) {
		let free = true;
		const nx = e.x + vx * dt;
		if (this.canStand(e.level, nx, e.y, e.radius)) e.x = nx;
		else if (Math.abs(vx) > 1e-4) free = false;
		const ny = e.y + vy * dt;
		if (this.canStand(e.level, e.x, ny, e.radius)) e.y = ny;
		else if (Math.abs(vy) > 1e-4) free = false;
		return free;
	}

	teleportTo(e, level, x, y) {
		e.level = level;
		e.x = x;
		e.y = y;
		e.stairsArmed = false;
		e.stairT = 0;
		if (e.carrying) {
			const c = this.chests.find((ch) => ch.id === e.carrying);
			if (c) {
				c.level = level;
				c.x = x;
				c.y = y;
			}
		}
		if (e.isPlayer) this.updateVision(true);
	}

	useStairs(e) {
		const tx = Math.round(e.x);
		const ty = Math.round(e.y);
		const tgt = stairsTarget(this.dungeon, e.level, tx, ty);
		if (!tgt) return false;
		const going = tgt.level > e.level ? 'down' : 'up';
		this.teleportTo(e, tgt.level, tgt.x, tgt.y);
		this.emit('stairs', { entity: e, level: tgt.level, going });
		if (e.isPlayer) this.say(going === 'down' ? `You descend to depth ${tgt.level + 1}` : `You climb to depth ${tgt.level + 1}`, 2.5);
		return true;
	}

	// ------------------------------------------------------------------ combat
	hostile(a, b) {
		return a.team !== b.team;
	}

	fire(e, angle) {
		if (e.cd > 0 || e.shots >= e.maxShots || !e.alive) return false;
		const mdef = e.kind === 'hero' ? e.base.missile : e.base.missile;
		const info = MISSILES[mdef];
		const dx = Math.cos(angle);
		const dy = Math.sin(angle);
		let image = mdef;
		if (e.type === 'ogre') image = this.rng.chance(0.5) ? 2 : 5;
		const m = {
			id: nextId++, level: e.level, x: e.x + dx * 0.4, y: e.y + dy * 0.4, vx: dx * (MISSILES[image].speed), vy: dy * (MISSILES[image].speed),
			owner: e.id, team: e.team, image, life: 1.6, dmg: e.dmg, kind: e.kind, ownerType: e.type, spin: 0,
		};
		void info;
		this.missiles.push(m);
		e.shots++;
		e.cd = e.kind === 'hero' ? e.fireCd : e.base.shotCd * this.rng.float(0.8, 1.25);
		e.aim = angle;
		e.dir = spriteDir(dx, dy);
		e.flashT = 0.12;
		if (e.isPlayer) this.stats.shots++;
		this.emit('fire', { entity: e, missile: m });
		return true;
	}

	rollDamage(src) {
		const [a, b] = src.dmg;
		return a + this.rng.int(b - a + 1);
	}

	applyDamage(target, amount, source, opts = {}) {
		if (!target.alive || amount <= 0) return false;
		if (target.protT > 0) {
			this.emit('blocked', { entity: target });
			return false;
		}
		let hurt = amount;
		if (target.armour > 0) {
			const soak = Math.min(target.armour, hurt);
			target.armour -= soak;
			hurt -= soak;
			if (soak > 0) this.emit('armourHit', { entity: target, amount: soak });
		}
		target.hp -= hurt;
		target.flashT = 0.18;
		if (target.asleep) this.wake(target);
		if (target.kind === 'hero') this.stats.damageTaken += target.isPlayer ? amount : 0;
		if (target.carrying && !opts.silent && this.rng.chance(0.55)) this.dropChest(target);
		this.emit('hit', { entity: target, amount, hurt, source, contact: !!opts.contact });
		if (target.hp <= 0) this.killEntity(target, source);
		return true;
	}

	wake(e, silent = false) {
		if (!e.asleep) return;
		e.asleep = false;
		if (!silent && e.kind === 'monster') this.emit('wake', { entity: e });
		if (e.kind === 'hero') this.emit('wake', { entity: e });
	}

	killEntity(e, source) {
		if (!e.alive) return;
		e.alive = false;
		e.hp = 0;
		e.deathT = 0;
		e.vx = e.vy = 0;
		if (e.carrying) this.dropChest(e);
		const killer = source && this.entities.find((x) => x.id === source.id);
		if (e.kind === 'monster') {
			this.stats.kills++;
			if (killer && killer.kind === 'hero') this.giveXp(killer, e.base.xp);
			const k = tileAt(this.dungeon, e.level, Math.round(e.x), Math.round(e.y));
			const key = this.idx(Math.round(e.x), Math.round(e.y));
			if (k === T.FLOOR && !this.items[e.level].has(key) && this.itemCount() < TUNING.itemCap) {
				const r = this.rng.int(4);
				const type = r === 0 ? 'armour' : r === 1 ? 'scroll' : r === 2 ? 'elixir' : null;
				if (type) this.items[e.level].set(key, { type, bob: 0, drop: true });
			}
			e.respawn = this.rng.float(...TUNING.monsterRespawn);
			this.emit('death', { entity: e, killer });
		} else {
			if (e.isPlayer) {
				this.stats.deaths++;
				e.respawn = TUNING.heroRespawn + 1.4;
			} else {
				e.respawn = TUNING.allyRespawn;
				if (killer && killer.kind === 'hero') this.giveXp(killer, 12);
			}
			this.emit('death', { entity: e, killer });
		}
	}

	itemCount() {
		let n = 0;
		for (const m of this.items) n += m.size;
		return n;
	}

	giveXp(hero, amount) {
		if (!hero.isPlayer) return;
		hero.xp += amount;
		this.emit('xp', { entity: hero, amount });
		while (hero.xp >= hero.nextXp) {
			hero.lvl++;
			hero.levelXp = hero.xp;
			hero.nextXp = hero.xp + xpForLevel(hero.lvl);
			hero.maxHp += 8;
			hero.hp = Math.min(hero.maxHp + 10, hero.hp + 20);
			let msg = '';
			switch (hero.attrib) {
				case 0: hero.maxShots = Math.min(6, hero.maxShots + 1); msg = 'more missiles in flight'; break;
				case 1: hero.dmg[1] = Math.min(6, hero.dmg[1] + 1); msg = 'higher maximum damage'; break;
				case 2: hero.fireCd = Math.max(0.14, hero.fireCd * 0.88); msg = 'faster fire rate'; break;
				default: hero.dmg[0] = Math.min(4, hero.dmg[0] + 1); msg = 'higher minimum damage'; break;
			}
			hero.attrib = (hero.attrib + 1) % 4;
			this.emit('levelup', { entity: hero, level: hero.lvl, msg });
			this.say(`Level ${hero.lvl}! You gain ${msg}.`, 5, 'good');
		}
	}

	// ------------------------------------------------------------------ chests
	chestById(id) {
		return this.chests.find((c) => c.id === id);
	}

	dropChest(e) {
		const c = this.chests.find((ch) => ch.id === e.carrying);
		if (!c) {
			e.carrying = null;
			return;
		}
		c.carrier = null;
		c.level = e.level;
		c.x = Math.round(e.x);
		c.y = Math.round(e.y);
		c.noPickup = 1.6;
		c.lastCarrier = e.id;
		e.carrying = null;
		this.emit('chestDropped', { chest: c, entity: e });
		if (e.isPlayer) this.say('You dropped the treasure!', 3, 'warn');
	}

	returnChest(c) {
		c.carrier = null;
		c.level = c.home.level;
		c.x = c.home.x;
		c.y = c.home.y;
		this.emit('chestReturned', { chest: c });
		this.say(`The ${c.color} chest returns to its resting place.`, 3);
	}

	updateChests(dt) {
		for (const c of this.chests) {
			c.bob += dt;
			if (c.noPickup > 0) c.noPickup -= dt;
			if (c.carrier) {
				const e = this.entities.find((x) => x.id === c.carrier);
				if (!e || !e.alive) {
					c.carrier = null;
					continue;
				}
				c.level = e.level;
				c.x = e.x;
				c.y = e.y;
				const pad = this.pads.find((p) => p.team === e.team);
				if (pad && pad.level === e.level && Math.hypot(pad.x - e.x, pad.y - e.y) < 0.75) {
					this.chestDelivered(c, e);
				}
				continue;
			}
			if (c.noPickup > 0) continue;
			for (const e of this.entities) {
				if (!e.alive || e.kind !== 'hero' || e.level !== c.level || e.asleep) continue;
				if (Math.hypot(e.x - c.x, e.y - c.y) > 0.65) continue;
				if (e.team === c.team) {
					if (c.x !== c.home.x || c.y !== c.home.y || c.level !== c.home.level) this.returnChest(c);
					continue;
				}
				if (e.carrying) continue;
				if (c.id === 'B' && !e.isPlayer) continue;
				c.carrier = e.id;
				e.carrying = c.id;
				this.emit('chestTaken', { chest: c, entity: e });
				if (e.isPlayer) this.say(c.id === 'B' ? 'You seize the Blue Chest! Carry it back to the stairs where you began!' : 'You carry the Red Chest!', 5, 'good');
				else this.say('An enemy has taken your Red Chest!', 4, 'bad');
				if (c.id === 'B' && !this.alarm) this.raiseAlarm();
				break;
			}
		}
	}

	raiseAlarm() {
		this.alarm = true;
		let n = 0;
		for (const e of this.entities) {
			if (e.asleep && e.alive) {
				this.wake(e, n++ > 3);
			}
		}
		this.emit('alarm', {});
		this.say('The dungeon awakens... everything is hunting you!', 5, 'bad');
	}

	chestDelivered(c, e) {
		c.carrier = null;
		e.carrying = null;
		if (e.isPlayer && c.id === 'B') {
			this.finish('won');
		} else if (!e.isPlayer && c.id === 'A') {
			this.finish('lost');
		}
	}

	finish(result) {
		if (this.state !== 'playing') return;
		this.state = result;
		const p = this.player;
		if (result === 'won') {
			p.xp += 30;
			p.armour += 10;
			p.rings += 2;
			p.hp = Math.min(p.maxHp + 10, p.hp + 20);
			this.giveXp(p, 0);
		}
		this.emit(result, { stats: this.stats, time: this.time });
	}

	// ------------------------------------------------------------------ pickups
	updatePickups(e) {
		const L = e.level;
		const tx = Math.round(e.x);
		const ty = Math.round(e.y);
		const key = this.idx(tx, ty);
		const it = this.items[L].get(key);
		if (!it) return;
		if (Math.hypot(e.x - tx, e.y - ty) > 0.7) return;
		const p = e;
		switch (it.type) {
			case 'elixir': {
				const cap = p.maxHp + 10;
				if (p.hp >= cap) return;
				p.hp = Math.min(cap, p.hp + TUNING.elixirHeal);
				this.emit('pickup', { entity: e, item: it, x: tx, y: ty, text: `+${TUNING.elixirHeal} Health` });
				break;
			}
			case 'armour': {
				const fit = p.type === 'fighter' || p.type === 'thief';
				const a = fit ? 5 : 2;
				p.armour += a;
				this.emit('pickup', { entity: e, item: it, x: tx, y: ty, text: `+${a} Armour` });
				break;
			}
			case 'scroll': {
				const fit = p.type === 'wizard' || p.type === 'cleric';
				const a = fit ? 5 : 2;
				p.armour += a;
				this.emit('pickup', { entity: e, item: it, x: tx, y: ty, text: `+${a} Ward` });
				break;
			}
			case 'ring':
				p.rings++;
				this.emit('pickup', { entity: e, item: it, x: tx, y: ty, text: '+1 Ring' });
				break;
		}
		this.items[L].delete(key);
		if (e.isPlayer) this.stats.items++;
	}

	// -------------------------------------------------------------------- ring
	useRing(e) {
		if (!e.alive || e.rings <= 0 || e.protT > 0) return false;
		e.rings--;
		e.protT = TUNING.ringDuration;
		this.emit('ring', { entity: e });
		return true;
	}

	// ------------------------------------------------------------------ vision
	updateVision(force = false) {
		const p = this.player;
		const tx = Math.round(p.x);
		const ty = Math.round(p.y);
		const vt = this.visTile;
		if (!force && vt.level === p.level && vt.x === tx && vt.y === ty) return;
		vt.level = p.level;
		vt.x = tx;
		vt.y = ty;
		this.visible = floodVisible(this.dungeon, p.level, tx, ty, TUNING.visionRadius);
		const ex = this.explored[p.level];
		const { w, h } = this.dungeon;
		for (let i = 0; i < this.visible.length; i++) {
			if (this.visible[i] >= 0) ex[i] = 1;
		}
		// reveal walls around visible tiles
		void w;
		void h;
		this.emit('vision', {});
	}

	isVisible(level, x, y) {
		const p = this.player;
		if (level !== p.level) return false;
		const tx = Math.round(x);
		const ty = Math.round(y);
		return this.visible[ty * this.dungeon.w + tx] >= 0;
	}

	// ------------------------------------------------------------------ update
	update(dt, input) {
		if (this.state !== 'playing') {
			this.updateEffectsOnly(dt);
			return;
		}
		dt = Math.min(dt, 0.05);
		this.time += dt;
		const p = this.player;

		this.updatePlayer(dt, input);
		for (const e of this.entities) {
			if (e.isPlayer) continue;
			this.updateEntity(e, dt);
		}
		this.updateMissiles(dt);
		this.updateContact(dt);
		this.updateChests(dt);
		this.updatePits(dt);

		for (const e of this.entities) {
			if (!e.alive) {
				e.deathT += dt;
				if (e.respawn > 0) {
					e.respawn -= dt;
					if (e.respawn <= 0) this.respawn(e);
				}
				continue;
			}
			e.cd = Math.max(0, e.cd - dt);
			e.hitT = Math.max(0, e.hitT - dt);
			e.protT = Math.max(0, e.protT - dt);
			e.slowT = Math.max(0, e.slowT - dt);
			e.flashT = Math.max(0, e.flashT - dt);
			e.noChestT = Math.max(0, e.noChestT - dt);
			if (e.kind === 'hero' && e.type === 'cleric' && e.hp < e.maxHp) e.hp = Math.min(e.maxHp, e.hp + dt * 0.45);
		}
		void p;
		this.updateVision();
	}

	updateEffectsOnly(dt) {
		for (const e of this.entities) {
			if (!e.alive) e.deathT += dt;
			e.flashT = Math.max(0, e.flashT - dt);
		}
	}

	respawn(e) {
		e.alive = true;
		e.hp = e.maxHp;
		e.deathT = 0;
		e.shots = 0;
		e.cd = 1;
		e.slowT = 0;
		e.hitT = 1.2;
		e.protT = e.kind === 'hero' ? 1.5 : 0;
		if (e.kind === 'monster') {
			e.level = e.home.level;
			e.x = e.home.x;
			e.y = e.home.y;
			e.asleep = !this.alarm;
		} else {
			e.level = e.home.level;
			e.x = e.home.x;
			e.y = e.home.y;
			e.asleep = false;
			if (e.isPlayer) {
				e.armour = 0;
				e.rings = Math.max(1, Math.floor(e.rings / 2));
				e.hp = e.maxHp;
				this.updateVision(true);
			}
		}
		e.stairsArmed = false;
		this.emit('respawn', { entity: e });
	}

	updatePlayer(dt, input) {
		const p = this.player;
		if (!p.alive) return;
		let mx = input.moveX || 0;
		let my = input.moveY || 0;
		const m = Math.hypot(mx, my);
		const slow = p.slowT > 0 ? 0.6 : 1;
		if (m > 0.05) {
			mx /= Math.max(1, m);
			my /= Math.max(1, m);
			p.vx = mx * p.speed * slow;
			p.vy = my * p.speed * slow;
			this.moveEntity(p, p.vx, p.vy, dt);
			p.moving = true;
			p.animT += dt * (p.speed / 4.4);
			p.lastMove = Math.atan2(my, mx);
			if (input.aim == null) p.dir = spriteDir(mx, my);
		} else {
			p.vx = p.vy = 0;
			p.moving = false;
		}
		if (input.aim != null) {
			p.aim = input.aim;
			p.dir = spriteDir(Math.cos(input.aim), Math.sin(input.aim));
		}
		if (input.fire) {
			let angle = input.aim != null ? input.aim : p.lastMove != null ? p.lastMove : p.aim;
			angle = this.assistAim(p, angle);
			this.fire(p, angle);
		}
		if (input.ring) this.useRing(p);
		if (input.drop && p.carrying) this.dropChest(p);
		if (p.hp <= p.maxHp * 0.25 && p.rings > 0 && p.protT <= 0 && !p.autoRingCd) {
			if (this.useRing(p)) this.say('Your ring flares to life!', 2.5, 'good');
		}
		this.handleStairs(p, dt, input.interact);
		this.updatePickups(p);
	}

	assistAim(e, angle) {
		let best = null;
		let bestDiff = 0.2;
		for (const t of this.entities) {
			if (!t.alive || t.level !== e.level || !this.hostile(e, t) || t.asleep) continue;
			const dx = t.x - e.x;
			const dy = t.y - e.y;
			const dist = Math.hypot(dx, dy);
			if (dist > 10 || dist < 0.3) continue;
			let diff = Math.atan2(dy, dx) - angle;
			diff = Math.atan2(Math.sin(diff), Math.cos(diff));
			if (Math.abs(diff) < bestDiff && lineClear(this.dungeon, e.level, e.x, e.y, t.x, t.y)) {
				bestDiff = Math.abs(diff);
				best = Math.atan2(dy, dx);
			}
		}
		return best != null ? best : angle;
	}

	handleStairs(e, dt, interact) {
		const tt = tileAt(this.dungeon, e.level, Math.round(e.x), Math.round(e.y));
		const onStairs = tt === T.DOWN || tt === T.UP;
		if (!onStairs) {
			e.stairsArmed = true;
			e.stairT = 0;
			return;
		}
		if (interact) {
			this.useStairs(e);
			return;
		}
		if (!e.stairsArmed) return;
		e.stairT += dt;
		if (e.isPlayer && e.stairT >= TUNING.stairsDelay && Math.hypot(e.x - Math.round(e.x), e.y - Math.round(e.y)) < 0.35) {
			this.useStairs(e);
		}
	}

	updatePits(dt) {
		for (const pit of this.pits) {
			pit.t = (pit.t + dt) % pit.period;
			const f = pit.t / pit.period;
			pit.open = f > 0.55 && f < 0.92 ? Math.min(1, Math.min((f - 0.55) / 0.08, (0.92 - f) / 0.08)) : 0;
			if (pit.open > 0.6) {
				for (const e of this.entities) {
					if (e.kind !== 'hero' || !e.alive || e.level !== pit.level) continue;
					if (Math.hypot(e.x - pit.x, e.y - pit.y) < 0.4) {
						this.teleportTo(e, pit.level + 1, pit.x, pit.y);
						e.hitT = 1;
						this.emit('fall', { entity: e, level: pit.level + 1 });
						if (e.isPlayer) this.say('You fall through a pit!', 3, 'warn');
						e.hp -= 2;
						e.flashT = 0.3;
						if (e.hp <= 0) this.killEntity(e, null);
					}
				}
			}
		}
	}

	// ---------------------------------------------------------------------- AI
	updateEntity(e, dt) {
		if (!e.alive) return;
		if (e.asleep) {
			this.checkWake(e);
			return;
		}
		e.aiT -= dt;
		if (e.kind === 'monster') this.thinkMonster(e, dt);
		else this.thinkHero(e, dt);
		const slow = e.slowT > 0 ? 0.55 : 1;
		const sp = Math.hypot(e.vx, e.vy);
		if (sp > 0.05) {
			this.moveEntity(e, e.vx * slow, e.vy * slow, dt);
			e.moving = true;
			e.animT += dt * Math.max(0.6, sp / 3.5);
		} else {
			e.moving = false;
		}
		this.pushApart(e, dt);
		if (e.kind === 'hero' && e.ai) this.updatePickups(e);
	}

	checkWake(e) {
		const p = this.player;
		if (!p.alive || p.level !== e.level) return;
		const d = Math.hypot(p.x - e.x, p.y - e.y);
		if (d < TUNING.wakeRadius) this.wake(e);
	}

	pushApart(e, dt) {
		for (const o of this.entities) {
			if (o === e || !o.alive || o.level !== e.level || o.team !== e.team) continue;
			const dx = e.x - o.x;
			const dy = e.y - o.y;
			const d = Math.hypot(dx, dy);
			const min = e.radius + o.radius;
			if (d > 0 && d < min) {
				const push = ((min - d) / min) * 2.2 * dt;
				this.moveEntity(e, (dx / d) * push * 20, (dy / d) * push * 20, dt);
			}
		}
	}

	nearestHostile(e, maxDist, needLos = true, onlyHeroes = false) {
		let best = null;
		let bd = maxDist;
		for (const t of this.entities) {
			if (!t.alive || t.level !== e.level || !this.hostile(e, t)) continue;
			if (onlyHeroes && t.kind !== 'hero') continue;
			if (t.asleep && e.kind === 'hero' && !t.asleep) continue;
			const d = Math.hypot(t.x - e.x, t.y - e.y);
			if (d < bd && (!needLos || lineClear(this.dungeon, e.level, e.x, e.y, t.x, t.y))) {
				bd = d;
				best = t;
			}
		}
		return best ? { target: best, dist: bd } : null;
	}

	thinkMonster(e, dt) {
		const base = e.base;
		e.bob += dt * 3;
		// find the closest hero on this level
		let tgt = null;
		let td = 1e9;
		for (const t of this.entities) {
			if (t.kind !== 'hero' || !t.alive || t.level !== e.level) continue;
			const d = Math.hypot(t.x - e.x, t.y - e.y);
			if (d < td) {
				td = d;
				tgt = t;
			}
		}
		let vx = 0;
		let vy = 0;
		if (tgt) {
			const los = lineClear(this.dungeon, e.level, e.x, e.y, tgt.x, tgt.y);
			const field = this.getField('h' + tgt.id, { level: tgt.level, x: Math.round(tgt.x), y: Math.round(tgt.y) }, false);
			const route = this.routeStep(e, field);
			const [pmin, pmax] = base.prefer;
			const dxT = tgt.x - e.x;
			const dyT = tgt.y - e.y;
			const dir = Math.hypot(dxT, dyT) || 1;
			if (los && td < base.range && e.cd <= 0 && e.shots < e.maxShots) {
				const jitter = (this.rng.next() - 0.5) * 0.18;
				this.fire(e, Math.atan2(dyT, dxT) + jitter);
			}
			if (pmax > 0 && los && td < pmin) {
				// back away
				vx = (-dxT / dir) * e.speed * 0.8;
				vy = (-dyT / dir) * e.speed * 0.8;
			} else if (pmax > 0 && los && td < pmax) {
				// strafe at preferred range
				const side = Math.sin(this.time * 0.8 + e.id) > 0 ? 1 : -1;
				vx = (-dyT / dir) * side * e.speed * 0.45;
				vy = (dxT / dir) * side * e.speed * 0.45;
			} else if (route && !route.arrived && td < 22) {
				vx = route.vx * e.speed;
				vy = route.vy * e.speed;
				if (route.stairs) this.monsterStairs(e);
			}
			if (los) e.aim = Math.atan2(dyT, dxT);
			const faceX = vx || dxT;
			const faceY = vy || dyT;
			if (Math.abs(faceX) + Math.abs(faceY) > 0.01) e.dir = spriteDir(faceX, faceY);
		}
		e.vx = vx;
		e.vy = vy;
	}

	monsterStairs() {}

	thinkHero(e, dt) {
		const goal = this.heroGoal(e);
		let vx = 0;
		let vy = 0;
		const eng = this.nearestHostile(e, e.team === 2 ? 8 : 7.5, true);
		if (eng) {
			const t = eng.target;
			const ang = Math.atan2(t.y - e.y, t.x - e.x);
			if (e.cd <= 0 && e.shots < e.maxShots) this.fire(e, ang + (this.rng.next() - 0.5) * 0.12);
			e.dir = spriteDir(Math.cos(ang), Math.sin(ang));
		}
		if (goal) {
			const field = this.getField('g' + e.id, goal);
			const route = this.routeStep(e, field);
			if (route && !route.arrived) {
				let sp = e.speed;
				if (goal.hold != null) {
					const dd = this.pathDist(field, e);
					if (dd >= 0 && dd <= goal.hold) sp = 0;
				}
				vx = route.vx * sp;
				vy = route.vy * sp;
				if (route.stairs) {
					e.stairsArmed = true;
					this.useStairs(e);
				}
				if (!eng && sp > 0) e.dir = spriteDir(vx, vy);
			}
		}
		if (eng && eng.dist < 3.2 && e.team === 1) {
			const t = eng.target;
			const dx = e.x - t.x;
			const dy = e.y - t.y;
			const d = Math.hypot(dx, dy) || 1;
			vx = (dx / d) * e.speed * 0.6;
			vy = (dy / d) * e.speed * 0.6;
		}
		e.vx = vx;
		e.vy = vy;
		if (e.carrying) {
			const pad = this.pads.find((p) => p.team === e.team);
			if (pad && pad.level === e.level) void pad;
		}
	}

	pathDist(field, e) {
		return field[e.level][Math.round(e.y) * this.dungeon.w + Math.round(e.x)];
	}

	heroGoal(e) {
		const p = this.player;
		if (e.team === 1) {
			if (!p.alive) return null;
			return { level: p.level, x: Math.round(p.x), y: Math.round(p.y), hold: 3 };
		}
		// enemy hero: fetch red chest, bring it home, harass the player when close
		const chestA = this.chestById('A');
		if (e.carrying) return { level: e.home.level, x: e.home.x, y: e.home.y };
		if (chestA && !chestA.carrier && e.noChestT <= 0) {
			return { level: chestA.level, x: Math.round(chestA.x), y: Math.round(chestA.y) };
		}
		if (p.alive && p.level === e.level && Math.hypot(p.x - e.x, p.y - e.y) < 12) {
			return { level: p.level, x: Math.round(p.x), y: Math.round(p.y), hold: 3 };
		}
		return { level: e.home.level, x: e.home.x, y: e.home.y, hold: 2 };
	}

	// ------------------------------------------------------------- projectiles
	updateMissiles(dt) {
		for (let i = this.missiles.length - 1; i >= 0; i--) {
			const m = this.missiles[i];
			const steps = Math.ceil((Math.hypot(m.vx, m.vy) * dt) / 0.2);
			let done = false;
			for (let s = 0; s < steps && !done; s++) {
				m.x += (m.vx * dt) / steps;
				m.y += (m.vy * dt) / steps;
				if (tileAt(this.dungeon, m.level, Math.round(m.x), Math.round(m.y)) === T.VOID) {
					this.missileImpact(m, null);
					done = true;
					break;
				}
				for (const e of this.entities) {
					if (!e.alive || e.level !== m.level || e.team === m.team || e.id === m.owner) continue;
					if (e.deathT > 0) continue;
					if (Math.hypot(e.x - m.x, e.y - m.y) < e.radius + 0.25) {
						this.missileImpact(m, e);
						done = true;
						break;
					}
				}
			}
			m.life -= dt;
			if (!done && m.life <= 0) {
				this.missileImpact(m, null, true);
				done = true;
			}
			if (done) {
				const owner = this.entities.find((x) => x.id === m.owner);
				if (owner) owner.shots = Math.max(0, owner.shots - 1);
				this.missiles.splice(i, 1);
			}
		}
	}

	missileImpact(m, target, expired = false) {
		const owner = this.entities.find((x) => x.id === m.owner);
		const info = MISSILES[m.image];
		this.emit('impact', { missile: m, target, expired, info });
		if (expired) return;
		const src = owner || { id: m.owner, dmg: m.dmg, team: m.team };
		if (info.aoe) {
			for (const e of this.entities) {
				if (!e.alive || e.level !== m.level || e.team === m.team) continue;
				const d = Math.hypot(e.x - m.x, e.y - m.y);
				if (d < info.aoe) {
					const dmg = Math.max(1, Math.round(this.rollDamage(m) * (e === target ? 1 : 0.6)));
					this.applyDamage(e, dmg, src);
				}
			}
		} else if (target) {
			let dmg = this.rollDamage(m);
			if (m.ownerType === 'thief' && this.rng.chance(0.18)) dmg += 2;
			this.applyDamage(target, dmg, src);
			if (info.slow && target.alive) target.slowT = 2.2;
		}
	}

	updateContact() {
		const p = this.player;
		for (const e of this.entities) {
			if (!e.alive || e.kind !== 'monster' || e.asleep || e.deathT > 0) continue;
			for (const h of this.entities) {
				if (!h.alive || h.kind !== 'hero' || h.level !== e.level) continue;
				if (Math.hypot(h.x - e.x, h.y - e.y) > h.radius + e.radius + 0.12) continue;
				if (h.protT > 0) {
					this.killEntity(e, h);
					this.emit('ringKill', { entity: e, hero: h });
					break;
				}
				if (h.hitT > 0) continue;
				h.hitT = TUNING.hitInvuln;
				this.applyDamage(h, this.rollDamage(e), e, { contact: true });
			}
		}
		void p;
	}

	// --------------------------------------------------------------- save/load
	carryOver() {
		const p = this.player;
		return {
			maxHp: p.maxHp, hp: p.hp, armour: p.armour, rings: p.rings, xp: p.xp, lvl: p.lvl,
			nextXp: p.nextXp, levelXp: p.levelXp || 0, dmg: [...p.dmg], maxShots: p.maxShots, fireCd: p.fireCd, attrib: p.attrib,
		};
	}
}

export { ITEM_TYPES, MONSTER_ORDER };
