import * as THREE from 'three';
import { THEMES } from './config.js';

const base = import.meta.env.BASE_URL + 'assets/';

function loadImage(url) {
	return new Promise((resolve, reject) => {
		const img = new Image();
		img.onload = () => resolve(img);
		img.onerror = () => reject(new Error('failed to load ' + url));
		img.src = url;
	});
}

async function loadJson(url) {
	const r = await fetch(url);
	if (!r.ok) throw new Error('failed to load ' + url);
	return r.json();
}

export const SOUND_FILES = [
	'walk', 'drip2', 'orb', 'treasure', 'dead', 'win', 'item', 'throw', 'throw2', 'impact', 'spawn', 'die',
	'mouther', 'dragon', 'alive', 'skeleton', 'deathorb', 'fireball', 'ouch', 'button', 'tsword', 'bsword',
	'ewizard', 'tcleric', 'protection',
];

/**
 * Loads every texture used by the game. `onProgress(fraction)` is called as files arrive.
 */
export async function loadAssets(renderer, onProgress = () => {}) {
	const maxAniso = renderer.capabilities.getMaxAnisotropy();
	const loader = new THREE.TextureLoader();
	let done = 0;
	let total = 0;
	const tick = () => onProgress(Math.min(1, ++done / total));

	const tex = (url, { srgb = true, repeat = false, mip = true, nearest = false } = {}) => {
		total++;
		return new Promise((resolve, reject) => {
			loader.load(
				base + url,
				(t) => {
					if (srgb) t.colorSpace = THREE.SRGBColorSpace;
					t.anisotropy = Math.min(8, maxAniso);
					t.generateMipmaps = mip;
					t.minFilter = mip ? THREE.LinearMipmapLinearFilter : THREE.LinearFilter;
					t.magFilter = nearest ? THREE.NearestFilter : THREE.LinearFilter;
					if (repeat) t.wrapS = t.wrapT = THREE.RepeatWrapping;
					tick();
					resolve(t);
				},
				undefined,
				() => reject(new Error('failed to load ' + url)),
			);
		});
	};
	const json = (url) => {
		total++;
		return loadJson(base + url).then((j) => (tick(), j));
	};
	const image = (url) => {
		total++;
		return loadImage(base + url).then((i) => (tick(), i));
	};

	const a = { themes: {}, ui: {} };
	const tasks = [];
	// Sprite atlases: no mipmaps (cells sit side by side), crisp magnification for pixel art.
	tasks.push(tex('sprites/heroes.png', { mip: false }).then((t) => (a.heroes = t)));
	tasks.push(json('sprites/heroes.json').then((j) => (a.heroesMeta = j)));
	tasks.push(tex('sprites/items.png', { mip: false }).then((t) => (a.items = t)));
	tasks.push(json('sprites/items.json').then((j) => (a.itemsMeta = j)));
	tasks.push(tex('sprites/splat.png', { mip: false }).then((t) => (a.splat = t)));
	tasks.push(json('themes/themes.json').then((j) => (a.themeMeta = j)));
	for (const name of THEMES) {
		tasks.push(
			Promise.all([
				tex(`themes/${name}_floor.png`, { repeat: true }),
				tex(`themes/${name}_wall.png`, { repeat: true }),
			]).then(([floor, wall]) => (a.themes[name] = { floor, wall })),
		);
	}
	for (const n of ['splash', 'scroll', 'portrait_thief', 'portrait_cleric', 'portrait_wizard', 'portrait_fighter', 'elixir']) {
		tasks.push(image(`ui/${n}.png`).then((i) => (a.ui[n] = i)));
	}
	await Promise.all(tasks);
	return a;
}

