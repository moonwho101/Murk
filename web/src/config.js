// Static game data, derived from the original Murk (1998) sources.

export const HEROES = {
	fighter: {
		id: 'fighter', image: 1, block: 'fighter', name: 'Barbarian', title: 'Master of Steel',
		blurb: 'Tough and straightforward. Heavy armour, hard-hitting thrown daggers.',
		hp: 46, armour: 8, speed: 4.3, dmg: [2, 4], fireCd: 0.34, maxShots: 3, missile: 1, pad: '#58e070',
		sound: 'bsword', stats: { Health: 4, Speed: 3, Power: 4, Magic: 1 },
	},
	thief: {
		id: 'thief', image: 3, block: 'thief', name: 'Thief', title: 'Shadow of the Deep',
		blurb: 'Fast and nimble. Rapid-fire daggers and a nose for loot.',
		hp: 38, armour: 3, speed: 5.1, dmg: [1, 3], fireCd: 0.22, maxShots: 4, missile: 1, pad: '#6aa8ff',
		sound: 'tsword', stats: { Health: 3, Speed: 5, Power: 3, Magic: 2 },
	},
	wizard: {
		id: 'wizard', image: 4, block: 'wizard', name: 'Mage', title: 'Keeper of Flame',
		blurb: 'Frail but deadly. Fireballs burst on impact and burn everything near.',
		hp: 32, armour: 0, speed: 4.4, dmg: [2, 4], fireCd: 0.46, maxShots: 3, missile: 3, pad: '#ff9a3c',
		sound: 'ewizard', stats: { Health: 2, Speed: 3, Power: 4, Magic: 5 },
	},
	cleric: {
		id: 'cleric', image: 5, block: 'cleric', name: 'Holy Man', title: 'Light in the Dark',
		blurb: 'Frost bolts chill enemies to a crawl. Slowly mends his own wounds.',
		hp: 40, armour: 2, speed: 4.4, dmg: [1, 3], fireCd: 0.3, maxShots: 4, missile: 4, pad: '#9ee8ff',
		sound: 'tcleric', stats: { Health: 3, Speed: 3, Power: 3, Magic: 4 },
	},
};

export const HERO_ORDER = ['fighter', 'cleric', 'thief', 'wizard'];

// Monster stat blocks: hp / damage from the original "ability" tables. `speedDiv`
// converts the episode "speed" number (higher = slower in the original) to tiles/sec.
export const MONSTERS = {
	beholder: {
		id: 'beholder', image: 2, block: 'beholder', col: 0, name: 'Beholder', hp: 30, dmg: [2, 4], baseSpeed: 3.6,
		missile: 3, shotCd: 1.5, maxShots: 3, range: 8, prefer: [3, 6], xp: 18, hover: true, wake: 'orb', staticFrame: true,
		glow: '#ff6a3a', radius: 0.36,
	},
	deathorb: {
		id: 'deathorb', image: 9, block: 'beholder', col: 1, name: 'Death Orb', hp: 50, dmg: [2, 6], baseSpeed: 3.0,
		missile: 4, shotCd: 0.7, maxShots: 8, range: 9, prefer: [3, 7], xp: 30, hover: true, wake: 'deathorb', staticFrame: true,
		glow: '#58b6ff', radius: 0.38,
	},
	skeleton: {
		id: 'skeleton', image: 6, block: 'skeleton', col: 0, name: 'Skeleton', hp: 10, dmg: [1, 2], baseSpeed: 3.8,
		missile: 1, shotCd: 2.0, maxShots: 2, range: 6, prefer: [0, 2], xp: 5, wake: 'skeleton', radius: 0.3,
	},
	mouther: {
		id: 'mouther', image: 7, block: 'mouther', col: 0, name: 'Gibbering Mouther', hp: 6, dmg: [1, 1], baseSpeed: 4.2,
		missile: 6, shotCd: 1.6, maxShots: 2, range: 5, prefer: [0, 3], xp: 3, wake: 'mouther', radius: 0.3,
	},
	ogre: {
		id: 'ogre', image: 8, block: 'ogre', col: 0, name: 'Brute', hp: 18, dmg: [1, 3], baseSpeed: 3.7,
		missile: 5, shotCd: 1.8, maxShots: 3, range: 6, prefer: [0, 2], xp: 9, wake: 'spawn', radius: 0.34,
	},
};

export const MONSTER_ORDER = ['beholder', 'deathorb', 'skeleton', 'mouther', 'ogre'];

// 1 dagger, 2 axe, 3 fireball, 4 frost, 5 rock, 6 venom
export const MISSILES = {
	1: { name: 'dagger', speed: 10, glow: null, color: '#cfd8ff', spin: 0 },
	2: { name: 'axe', speed: 8.5, glow: null, color: '#ffb0a0', spin: 1 },
	3: { name: 'fireball', speed: 8, glow: '#ff8a2a', color: '#ffb060', aoe: 1.1 },
	4: { name: 'frost', speed: 8.5, glow: '#7fd4ff', color: '#aee6ff', slow: true },
	5: { name: 'rock', speed: 8, glow: null, color: '#aaaaaa', spin: 1 },
	6: { name: 'venom', speed: 9, glow: '#5cff7a', color: '#8bff9c' },
};

export const THEMES = ['murk', 'stone', 'moss', 'cave', 'ember', 'frost', 'ash', 'bone', 'lava'];

export const THEME_STYLE = {
	murk: { tint: '#ffffff', light: '#ffd9a8', ambient: '#34503a', fog: '#05080a' },
	stone: { tint: '#c9d0d8', light: '#ffb878', ambient: '#2a2e3c', fog: '#05060a' },
	moss: { tint: '#c7e2a8', light: '#ffc07a', ambient: '#243628', fog: '#040a06' },
	cave: { tint: '#e8c4a8', light: '#ff9a58', ambient: '#3a2a22', fog: '#0a0604' },
	ember: { tint: '#e8a49c', light: '#ff8048', ambient: '#3a2224', fog: '#0a0404' },
	frost: { tint: '#a8dce8', light: '#a8d8ff', ambient: '#1e3446', fog: '#03080c' },
	ash: { tint: '#c4c4cc', light: '#ffcf9a', ambient: '#2c2c34', fog: '#060608' },
	bone: { tint: '#e8e2d0', light: '#ffd9a0', ambient: '#3a3830', fog: '#080806' },
	lava: { tint: '#ffb898', light: '#ff6a2a', ambient: '#3c1c14', fog: '#0c0402' },
};

// Words substituted for the original in-text glyph codes ("-p" etc.).
export const TEXT_TOKENS = {
	a: 'Barbarian', b: 'Beholders', c: 'Thief', d: 'Mage', e: 'Holy Man', f: 'missiles',
	g: 'Skeletons', h: 'Mouthers', i: 'Brutes',
	k: 'Rune Circle', p: 'Blue Chest', u: 'Red Chest',
	q: 'Armour', s: 'Scrolls', t: 'Rings', x: 'Pits', y: 'Ladders',
};

export const TUNING = {
	heroRadius: 0.28,
	missileRadius: 0.3,
	wakeRadius: 4.6,
	ringDuration: 4,
	hitInvuln: 0.55,
	heroRespawn: 1.8,
	allyRespawn: 8,
	monsterRespawn: [26, 46],
	elixirHeal: 6,
	itemCap: 60,
	stairsDelay: 0.45,
	visionRadius: 7,
};

export function xpForLevel(level) {
	return Math.round(30 + level * 26 + level * level * 5);
}

export function monsterSpeedFromEpisode(base, epSpeed) {
	const s = Math.min(6, Math.max(1, epSpeed || 2));
	return base * (2 / (1 + s));
}
