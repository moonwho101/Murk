import episodes from './data/episodes.json';
import { loadAssets } from './assets.js';
import { GameView } from './renderer.js';
import { World } from './world.js';
import { UI } from './ui.js';
import { AudioSys } from './audio.js';
import { Input } from './input.js';
import { HEROES } from './config.js';

const params = new URLSearchParams(location.search);
const SAVE_KEY = 'murk.save.v1';

const state = {
	mode: 'loading', // title | select | briefing | playing | paused | result
	hero: null,
	episodeNo: 1,
	carry: null,
	startCarry: null,
	world: null,
	paused: false,
	resultTimer: 0,
};

const canvas = document.getElementById('game');
const view = new GameView(canvas);
const audio = new AudioSys();
const input = new Input(canvas);
let ui;
let loop = false;

function readSave() {
	try {
		return JSON.parse(localStorage.getItem(SAVE_KEY));
	} catch {
		return null;
	}
}

function writeSave(carry = state.carry) {
	localStorage.setItem(SAVE_KEY, JSON.stringify({ hero: state.hero, episodeNo: state.episodeNo, carry }));
}

// ------------------------------------------------------------------- flow
function showTitle() {
	state.mode = 'title';
	state.world = null;
	audio.ensureContext();
	ui.showTitle({
		hasSave: !!readSave(),
		muted: audio.muted,
		onMute: () => {
			audio.setMuted(!audio.muted);
			return audio.muted;
		},
		onNew: () => {
			audio.ensureContext();
			audio.startAmbience();
			audio.play('button');
			showSelect();
		},
		onContinue: () => {
			audio.ensureContext();
			audio.startAmbience();
			const s = readSave();
			if (!s) return;
			state.hero = s.hero;
			state.episodeNo = s.episodeNo;
			state.carry = s.carry;
			state.startCarry = s.carry;
			startEpisode();
		},
		onHelp: () => {
			audio.ensureContext();
			ui.showHelp({ onClose: showTitle });
		},
	});
}

function showSelect() {
	state.mode = 'select';
	ui.showSelect({
		onBack: showTitle,
		onPick: (hero) => {
			state.hero = hero;
			state.episodeNo = 1;
			state.carry = null;
			state.startCarry = null;
			audio.play(HEROES[hero].sound, { vol: 0.9 });
			startEpisode();
		},
	});
}

function episodeData() {
	const eps = episodes[state.hero];
	return eps[(state.episodeNo - 1) % eps.length];
}

function startEpisode(retry = false) {
	state.mode = 'briefing';
	state.world = null;
	ui.showBriefing({
		hero: state.hero,
		episodeNo: state.episodeNo,
		episode: episodeData(),
		retry,
		onBegin: beginPlay,
	});
}

function beginPlay(seed) {
	ui.clearScreen();
	audio.ensureContext();
	audio.startAmbience();
	const world = new World({
		hero: state.hero,
		episode: episodeData(),
		episodeNo: state.episodeNo,
		carry: state.startCarry,
		seed: typeof seed === 'number' ? seed : undefined,
	});
	state.world = world;
	input.leftPressed = false;
	state.paused = false;
	state.mode = 'playing';
	state.resultTimer = 0;
	view.setWorld(world);
	audio.bindWorld(world);
	if (audio.ctx) audio.setTheme((state.episodeNo - 1) % 4);
	ui.showHud(state.hero);
	ui.setEpisode(world);
	view.floatSpawner = (x, y, z, text, cls) => ui.float(x, y, z, text, cls);

	world.on((type, d) => {
		switch (type) {
			case 'message':
				ui.say(d.text, d.duration, d.kind);
				break;
			case 'xp':
				ui.float(d.entity.x, 1.4, d.entity.y, `+${d.amount} xp`, 'xp');
				break;
			case 'death':
				if (d.entity.isPlayer) ui.say('You have fallen... you will rise again at the rune circle.', 4, 'bad');
				break;
			case 'respawn':
				if (d.entity.isPlayer) ui.say('You return to the fight, stripped of your armour.', 3.5, 'warn');
				break;
			case 'won':
			case 'lost':
				state.resultTimer = type === 'won' ? 1.8 : 1.2;
				state.resultKind = type;
				break;
			default:
		}
	});
	window.__murk = { world, view, state, audio };
	ui.say(world.def.gametype === 2 ? 'Defend your Red Chest — and find the Blue one!' : 'Find the Blue Chest!', 5);
}

function finishEpisode(won) {
	state.mode = 'result';
	const world = state.world;
	ui.showResult({
		won,
		world,
		hero: state.hero,
		onNext: () => {
			state.carry = world.carryOver();
			state.startCarry = state.carry;
			state.episodeNo++;
			writeSave();
			audio.play('button');
			startEpisode();
		},
		onRetry: () => {
			audio.play('button');
			startEpisode(true);
		},
		onQuit: () => {
			if (won) {
				state.carry = world.carryOver();
				state.episodeNo++;
				writeSave();
			}
			showTitle();
		},
	});
}

function togglePause(force) {
	if (state.mode !== 'playing' && state.mode !== 'paused') return;
	const pause = force ?? state.mode === 'playing';
	if (pause) {
		state.mode = 'paused';
		ui.showPause({
			audio,
			onVolume: (s, m) => audio.setVolumes(s, m),
			onResume: () => togglePause(false),
			onHelp: () =>
				ui.showHelp({
					onClose: () => togglePause(true),
				}),
			onRestart: () => {
				ui.clearScreen();
				startEpisode(true);
			},
			onQuit: showTitle,
		});
	} else {
		state.mode = 'playing';
		ui.clearScreen();
	}
}

input.onKey = (e) => {
	if (e.code === 'Escape') {
		if (state.mode === 'playing' || state.mode === 'paused') togglePause();
		return;
	}
	if (state.mode === 'briefing' && (e.code === 'Enter' || e.code === 'Space')) {
		beginPlay();
		return;
	}
	if (state.mode === 'playing') {
		if (e.code === 'F3') {
			e.preventDefault();
			writeSave(state.world.carryOver());
			ui.say('Game saved.', 2.5, 'good');
		} else if (e.code === 'F4') {
			e.preventDefault();
			const s = readSave();
			if (s) {
				state.hero = s.hero;
				state.episodeNo = s.episodeNo;
				state.carry = s.carry;
				state.startCarry = s.carry;
				startEpisode();
			} else ui.say('There are no saved games...', 2.5, 'warn');
		} else if (e.code === 'F1') {
			e.preventDefault();
			togglePause(true);
			ui.showHelp({ onClose: () => togglePause(true) });
		} else if (e.code === 'KeyM') {
			audio.setMuted(!audio.muted);
		}
	}
};
input.onWheel = (d) => view.zoom(d);
document.addEventListener('visibilitychange', () => {
	if (document.hidden && state.mode === 'playing') togglePause(true);
});

// ------------------------------------------------------------------- loop
let last = performance.now();
let acc = 0;
const STEP = 1 / 60;

function frame(now) {
	requestAnimationFrame(frame);
	const dt = Math.min(0.1, (now - last) / 1000);
	last = now;
	const world = state.world;
	if (world) {
		if (state.mode === 'playing') {
			const inp = input.poll(view, world);
			acc += dt;
			let first = true;
			while (acc >= STEP) {
				// one-shot commands only go to the first sub-step
				world.update(STEP, first ? inp : { ...inp, ring: false, drop: false, interact: false });
				first = false;
				acc -= STEP;
			}
			audio.setListener(world.player.x, world.player.y);
			audio.update(dt, world);
			if (world.state !== 'playing' && state.resultTimer > 0) {
				state.resultTimer -= dt;
				if (state.resultTimer <= 0) finishEpisode(state.resultKind === 'won');
			}
		}
		const vdt = state.mode === 'paused' ? 0 : dt;
		view.update(vdt);
		view.render();
		ui.updateHud(world, view, vdt);
	} else if (view.world) {
		view.update(dt * 0.5);
		view.render();
	}
	input.endFrame();
}

// ------------------------------------------------------------------- boot
async function boot() {
	const bar = (f) => {
		const fill = document.getElementById('load-fill');
		if (fill) fill.style.width = Math.round(f * 100) + '%';
	};
	const assets = await loadAssets(view.renderer, bar);
	view.assets = assets;
	ui = new UI(assets);
	ui.hideLoading();
	loop = true;
	requestAnimationFrame(frame);

	const heroParam = params.get('hero');
	if (heroParam && HEROES[heroParam]) {
		state.hero = heroParam;
		state.episodeNo = Number(params.get('ep') || 1);
		state.carry = null;
		state.startCarry = null;
		beginPlay(params.has('seed') ? Number(params.get('seed')) : undefined);
	} else showTitle();
}

boot().catch((e) => {
	console.error(e);
	document.querySelector('.load-sub').textContent = 'Failed to load: ' + e.message;
});
void loop;
