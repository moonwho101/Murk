import { SOUND_FILES } from './assets.js';
import { GenMusic } from './music.js';

const base = import.meta.env.BASE_URL + 'assets/sound/';

/** Small WebAudio mixer: one-shot samples with distance attenuation + pan, looping ambience, drone. */
export class AudioSys {
	constructor() {
		this.ctx = null;
		this.buffers = {};
		this.muted = localStorage.getItem('murk.muted') === '1';
		this.sfxVol = 0.8;
		this.musicVol = 0.5;
		this.listener = { x: 0, y: 0 };
		this.loaded = false;
		this.nextDrip = 6;
		this.stepT = 0;
		this.music = null;
		this.theme = 0;
		this.gen = null;
		this.musicPlaying = false;
	}

	ensureContext() {
		if (this.ctx) {
			if (this.ctx.state === 'suspended') this.ctx.resume();
			return;
		}
		const Ctx = window.AudioContext || window.webkitAudioContext;
		if (!Ctx) return;
		this.ctx = new Ctx();
		this.master = this.ctx.createGain();
		this.master.gain.value = this.muted ? 0 : 1;
		this.master.connect(this.ctx.destination);
		this.sfxBus = this.ctx.createGain();
		this.sfxBus.gain.value = this.sfxVol;
		this.sfxBus.connect(this.master);
		this.musicBus = this.ctx.createGain();
		this.musicBus.gain.value = this.musicVol;
		this.musicBus.connect(this.master);
		if (!this.loaded) this.loadAll();
	}

	async loadAll() {
		this.loaded = true;
		await Promise.all(
			SOUND_FILES.map(async (n) => {
				try {
					const r = await fetch(`${base}${n}.wav`);
					const buf = await r.arrayBuffer();
					this.buffers[n] = await this.ctx.decodeAudioData(buf);
				} catch (e) {
					console.warn('sound failed', n, e);
				}
			}),
		);
		if (this.musicWanted) this.startAmbience();
	}

	setMuted(m) {
		this.muted = m;
		localStorage.setItem('murk.muted', m ? '1' : '0');
		if (this.master) this.master.gain.setTargetAtTime(m ? 0 : 1, this.ctx.currentTime, 0.05);
	}

	setVolumes(sfx, music) {
		this.sfxVol = sfx;
		this.musicVol = music;
		if (this.sfxBus) {
			this.sfxBus.gain.value = sfx;
			this.musicBus.gain.value = music;
		}
	}

	play(name, { vol = 1, rate = 1, x = null, y = null, jitter = 0.06 } = {}) {
		if (!this.ctx || this.muted) return null;
		const buf = this.buffers[name];
		if (!buf) return null;
		let gain = vol;
		let pan = 0;
		if (x != null) {
			const dx = x - this.listener.x;
			const dy = y - this.listener.y;
			const d = Math.hypot(dx, dy);
			gain *= Math.max(0, 1 - d / 16) ** 1.4;
			if (gain < 0.01) return null;
			// screen-space horizontal offset in the iso view
			pan = Math.max(-1, Math.min(1, ((dx - dy) * Math.SQRT1_2) / 9));
		}
		const src = this.ctx.createBufferSource();
		src.buffer = buf;
		src.playbackRate.value = rate * (1 + (Math.random() - 0.5) * jitter * 2);
		const g = this.ctx.createGain();
		g.gain.value = gain;
		src.connect(g);
		let node = g;
		if (this.ctx.createStereoPanner) {
			const p = this.ctx.createStereoPanner();
			p.pan.value = pan;
			g.connect(p);
			node = p;
		}
		node.connect(this.sfxBus);
		src.start();
		return src;
	}

	// ---------------------------------------------------------------- music
	startAmbience(theme = this.theme) {
		this.musicWanted = true;
		this.theme = theme;
		if (!this.ctx) return;
		if (!this.gen) this.gen = new GenMusic(this.ctx, this.musicBus);
		if (!this.musicPlaying) {
			this.gen.start(theme);
			this.musicPlaying = true;
		}
	}

	/** Switch the harmonic theme (one per episode); restarts the generator. */
	setTheme(theme) {
		if (theme === this.theme && this.musicPlaying) return;
		this.theme = theme;
		if (this.musicPlaying) {
			this.gen.start(theme);
		}
	}

	setIntensity(v) {
		if (this.gen) this.gen.setIntensity(v);
	}

	stopAmbience() {
		this.musicWanted = false;
		if (this.gen) this.gen.stop();
		this.musicPlaying = false;
	}

	setListener(x, y) {
		this.listener.x = x;
		this.listener.y = y;
	}

	/** Danger level 0..1 drives the adaptive score. */
	dangerLevel(world) {
		const p = world.player;
		let near = 0;
		for (const e of world.entities) {
			if (!e.alive || e.asleep || e.level !== p.level || e.team === p.team) continue;
			const d = Math.hypot(e.x - p.x, e.y - p.y);
			if (d < 9) near += 1 - d / 9;
		}
		let v = 0.1 + Math.min(0.75, near * 0.28);
		if (p.carrying) v = Math.max(v, 0.55);
		if (world.alarm) v = Math.max(v, 0.4);
		if (!p.alive) v = 0.05;
		return Math.min(1, v);
	}

	update(dt, world) {
		this.setIntensity(this.dangerLevel(world));
		if (!this.ctx || this.muted) return;
		const p = world.player;
		if (p.alive && p.moving) {
			this.stepT -= dt;
			if (this.stepT <= 0) {
				this.stepT = 0.3 / Math.max(0.8, p.speed / 4.4);
				this.play('walk', { vol: 0.35, rate: 0.9 + Math.random() * 0.2 });
			}
		} else this.stepT = 0.05;
		this.nextDrip -= dt;
		if (this.nextDrip <= 0) {
			this.nextDrip = 5 + Math.random() * 11;
			this.play('drip2', { vol: 0.28, rate: 0.9 + Math.random() * 0.3 });
		}
	}

	/** Wire sound effects to world events. */
	bindWorld(world) {
		const A = this;
		world.on((type, d) => {
			const e = d.entity;
			switch (type) {
				case 'fire': {
					const big = ['beholder', 'deathorb', 'wizard', 'cleric'].includes(e.type);
					A.play(big ? 'fireball' : e.type === 'ogre' ? 'throw2' : 'throw', { vol: big ? 0.6 : 0.8, x: e.x, y: e.y, jitter: 0.1 });
					break;
				}
				case 'hit':
					if (e.isPlayer) A.play('ouch', { vol: 0.9 });
					A.play('impact', { vol: 0.8, x: e.x, y: e.y });
					break;
				case 'impact':
					if (!d.target && !d.expired) A.play('impact', { vol: 0.4, x: d.missile.x, y: d.missile.y, rate: 1.4 });
					break;
				case 'death':
					if (e.isPlayer) A.play('dead', { vol: 1 });
					else A.play('die', { vol: 1, x: e.x, y: e.y, rate: e.kind === 'hero' ? 0.8 : 1 });
					break;
				case 'wake': {
					const map = { beholder: 'orb', deathorb: 'deathorb', skeleton: 'skeleton', mouther: 'mouther', ogre: 'spawn' };
					A.play(map[e.type] || 'spawn', { vol: 0.8, x: e.x, y: e.y });
					break;
				}
				case 'pickup':
					A.play('item', { vol: 0.9 });
					break;
				case 'chestTaken':
					A.play('treasure', { vol: 1 });
					break;
				case 'chestDropped':
					A.play('button', { vol: 0.8 });
					break;
				case 'chestReturned':
					A.play('alive', { vol: 0.7 });
					break;
				case 'ring':
					A.play('protection', { vol: 1 });
					break;
				case 'levelup':
					A.play('tcleric', { vol: 0.7 });
					break;
				case 'respawn':
					if (e.isPlayer) A.play('alive', { vol: 0.9 });
					break;
				case 'stairs':
					if (e.isPlayer) A.play('button', { vol: 0.9, rate: 0.8 });
					break;
				case 'fall':
					if (e.isPlayer) A.play('dead', { vol: 0.6, rate: 1.4 });
					break;
				case 'alarm':
					A.play('dragon', { vol: 0.9 });
					break;
				case 'won':
					A.play('win', { vol: 1 });
					break;
				case 'lost':
					A.play('dead', { vol: 1, rate: 0.7 });
					break;
				case 'ringKill':
					A.play('fireball', { vol: 0.7, x: d.entity.x, y: d.entity.y, rate: 0.7 });
					break;
				default:
			}
		});
	}
}

