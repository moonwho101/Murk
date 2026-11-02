// Procedural, adaptive dungeon score built on WebAudio (no samples).
// Layers: evolving pad, sub bass, plucked arpeggio and drums. `intensity` (0..1) fades the
// layers in as danger rises, so exploration is eerie ambience and combat turns into a driving synth groove.

const MODES = {
	aeolian: [0, 2, 3, 5, 7, 8, 10],
	phrygian: [0, 1, 3, 5, 7, 8, 10],
	dorian: [0, 2, 3, 5, 7, 9, 10],
	harmonicMinor: [0, 2, 3, 5, 7, 8, 11],
};

const THEMES = [
	{ root: 45, mode: 'aeolian', bpm: 78, prog: [0, 5, 2, 6] },
	{ root: 43, mode: 'phrygian', bpm: 84, prog: [0, 1, 0, 6] },
	{ root: 40, mode: 'dorian', bpm: 90, prog: [0, 3, 0, 4] },
	{ root: 47, mode: 'harmonicMinor', bpm: 80, prog: [0, 5, 3, 4] },
];

function mtof(n) {
	return 440 * Math.pow(2, (n - 69) / 12);
}

export class GenMusic {
	constructor(ctx, dest) {
		this.ctx = ctx;
		this.out = ctx.createGain();
		this.out.gain.value = 0.9;
		this.out.connect(dest);

		// reverb: synthetic exponentially decaying noise impulse
		this.reverb = ctx.createConvolver();
		const len = Math.floor(ctx.sampleRate * 3.2);
		const ir = ctx.createBuffer(2, len, ctx.sampleRate);
		for (let c = 0; c < 2; c++) {
			const d = ir.getChannelData(c);
			for (let i = 0; i < len; i++) d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / len, 2.6);
		}
		this.reverb.buffer = ir;
		const wet = ctx.createGain();
		wet.gain.value = 0.55;
		this.reverb.connect(wet);
		wet.connect(this.out);
		this.dry = ctx.createGain();
		this.dry.connect(this.out);
		this.dry.connect(this.reverb);

		// ping-pong-ish echo for the arpeggio
		this.delay = ctx.createDelay(1);
		this.delay.delayTime.value = 0.375;
		const fb = ctx.createGain();
		fb.gain.value = 0.38;
		const dlp = ctx.createBiquadFilter();
		dlp.type = 'lowpass';
		dlp.frequency.value = 2400;
		this.delay.connect(dlp);
		dlp.connect(fb);
		fb.connect(this.delay);
		dlp.connect(this.reverb);
		dlp.connect(this.out);

		this.layers = {};
		for (const name of ['pad', 'bass', 'arp', 'drums']) {
			const g = ctx.createGain();
			g.gain.value = 0;
			g.connect(this.dry);
			this.layers[name] = g;
		}
		this.arpSend = ctx.createGain();
		this.arpSend.gain.value = 0.7;
		this.arpSend.connect(this.delay);
		this.layers.arp.connect(this.arpSend);

		this.noise = ctx.createBuffer(1, ctx.sampleRate, ctx.sampleRate);
		const nd = this.noise.getChannelData(0);
		for (let i = 0; i < nd.length; i++) nd[i] = Math.random() * 2 - 1;

		this.intensity = 0.1;
		this.target = 0.1;
		this.timer = null;
		this.theme = THEMES[0];
	}

	start(themeIndex = 0) {
		this.stop();
		this.theme = THEMES[themeIndex % THEMES.length];
		this.step = 0;
		this.nextTime = this.ctx.currentTime + 0.2;
		this.timer = setInterval(() => this.tick(), 100);
	}

	stop() {
		if (this.timer) clearInterval(this.timer);
		this.timer = null;
	}

	setIntensity(v) {
		this.target = Math.max(0, Math.min(1, v));
	}

	scaleNote(degree, octave = 0) {
		const t = this.theme;
		const sc = MODES[t.mode];
		const oct = Math.floor(degree / sc.length);
		const idx = ((degree % sc.length) + sc.length) % sc.length;
		return t.root + sc[idx] + 12 * (oct + octave);
	}

	tick() {
		const ctx = this.ctx;
		// slew the intensity: rises quickly, falls slowly
		const k = this.target > this.intensity ? 0.35 : 0.06;
		this.intensity += (this.target - this.intensity) * k * 0.1;
		const I = this.intensity;
		const L = this.layers;
		const now = ctx.currentTime;
		L.pad.gain.setTargetAtTime(0.5 - I * 0.12, now, 0.6);
		L.bass.gain.setTargetAtTime(0.1 + I * 0.5, now, 0.6);
		L.arp.gain.setTargetAtTime(0.05 + Math.max(0, I - 0.15) * 0.55, now, 0.6);
		L.drums.gain.setTargetAtTime(I > 0.3 ? 0.25 + I * 0.7 : 0, now, 0.4);

		const spb = 60 / this.theme.bpm;
		const stepLen = spb / 4;
		while (this.nextTime < now + 0.5) {
			this.schedule(this.step, this.nextTime, stepLen);
			this.nextTime += stepLen;
			this.step++;
		}
	}

	schedule(step, t, stepLen) {
		const I = this.intensity;
		const bar = Math.floor(step / 16);
		const s16 = step % 16;
		const chord = this.theme.prog[Math.floor(bar / 2) % this.theme.prog.length];

		if (s16 === 0 && bar % 2 === 0) this.pad(chord, t, stepLen * 32);
		if (s16 % 2 === 0) this.bass(chord, s16, t, stepLen, I);
		this.arp(chord, step, t, stepLen, I);
		if (I > 0.3) this.drumStep(s16, bar, t, I);
	}

	pad(chord, t, dur) {
		const ctx = this.ctx;
		const g = ctx.createGain();
		const lp = ctx.createBiquadFilter();
		lp.type = 'lowpass';
		lp.frequency.setValueAtTime(500, t);
		lp.frequency.linearRampToValueAtTime(1400 + this.intensity * 1400, t + dur * 0.5);
		lp.frequency.linearRampToValueAtTime(500, t + dur);
		g.gain.setValueAtTime(0, t);
		g.gain.linearRampToValueAtTime(0.16, t + dur * 0.35);
		g.gain.linearRampToValueAtTime(0, t + dur + 0.6);
		lp.connect(g);
		g.connect(this.layers.pad);
		for (const off of [0, 2, 4, 7]) {
			const n = this.scaleNote(chord + off, 1);
			for (const det of [-7, 7]) {
				const o = ctx.createOscillator();
				o.type = 'sawtooth';
				o.frequency.value = mtof(n);
				o.detune.value = det;
				o.connect(lp);
				o.start(t);
				o.stop(t + dur + 0.7);
			}
		}
	}

	bass(chord, s16, t, stepLen, I) {
		if (I < 0.12 && s16 !== 0) return;
		const ctx = this.ctx;
		const pattern = [1, 0, 0, 1, 0, 1, 0, 0];
		if (!pattern[s16 / 2] && s16 !== 0) return;
		const note = this.scaleNote(chord, -2) + (s16 === 6 && I > 0.5 ? 12 : 0);
		const o = ctx.createOscillator();
		const o2 = ctx.createOscillator();
		const g = ctx.createGain();
		const lp = ctx.createBiquadFilter();
		lp.type = 'lowpass';
		lp.Q.value = 6;
		lp.frequency.setValueAtTime(300 + I * 1600, t);
		lp.frequency.exponentialRampToValueAtTime(110, t + stepLen * 1.8);
		o.type = 'sawtooth';
		o2.type = 'sine';
		o.frequency.value = mtof(note);
		o2.frequency.value = mtof(note - 12);
		g.gain.setValueAtTime(0.0001, t);
		g.gain.exponentialRampToValueAtTime(0.5, t + 0.01);
		g.gain.exponentialRampToValueAtTime(0.001, t + stepLen * 2.2);
		o.connect(lp);
		o2.connect(g);
		lp.connect(g);
		g.connect(this.layers.bass);
		o.start(t);
		o2.start(t);
		o.stop(t + stepLen * 2.4);
		o2.stop(t + stepLen * 2.4);
	}

	arp(chord, step, t, stepLen, I) {
		const s16 = step % 16;
		const density = I < 0.2 ? [0, 6, 10] : I < 0.55 ? [0, 3, 6, 8, 11, 14] : [0, 2, 3, 6, 8, 10, 11, 14];
		if (!density.includes(s16)) return;
		const shape = [0, 2, 4, 7, 4, 2, 9, 7];
		const deg = chord + shape[(step >> 1) % shape.length];
		const note = this.scaleNote(deg, 1 + ((step >> 4) % 2));
		const ctx = this.ctx;
		const o = ctx.createOscillator();
		const g = ctx.createGain();
		const lp = ctx.createBiquadFilter();
		lp.type = 'lowpass';
		lp.frequency.setValueAtTime(3200, t);
		lp.frequency.exponentialRampToValueAtTime(500, t + 0.25);
		o.type = I > 0.5 ? 'sawtooth' : 'triangle';
		o.frequency.value = mtof(note);
		g.gain.setValueAtTime(0.0001, t);
		g.gain.exponentialRampToValueAtTime(0.32, t + 0.005);
		g.gain.exponentialRampToValueAtTime(0.001, t + 0.35);
		o.connect(lp);
		lp.connect(g);
		g.connect(this.layers.arp);
		o.start(t);
		o.stop(t + 0.4);
	}

	drumStep(s16, bar, t, I) {
		const ctx = this.ctx;
		const out = this.layers.drums;
		const kick = s16 === 0 || s16 === 8 || (I > 0.65 && (s16 === 6 || s16 === 14));
		const snare = s16 === 4 || s16 === 12;
		const hat = s16 % 2 === 0 || (I > 0.7 && s16 % 2 === 1);
		if (kick) {
			const o = ctx.createOscillator();
			const g = ctx.createGain();
			o.frequency.setValueAtTime(150, t);
			o.frequency.exponentialRampToValueAtTime(42, t + 0.14);
			g.gain.setValueAtTime(0.9, t);
			g.gain.exponentialRampToValueAtTime(0.001, t + 0.3);
			o.connect(g);
			g.connect(out);
			o.start(t);
			o.stop(t + 0.32);
		}
		if (snare || hat) {
			const src = ctx.createBufferSource();
			src.buffer = this.noise;
			const f = ctx.createBiquadFilter();
			const g = ctx.createGain();
			f.type = snare ? 'bandpass' : 'highpass';
			f.frequency.value = snare ? 1900 : 7500;
			const len = snare ? 0.2 : s16 % 4 === 2 ? 0.09 : 0.04;
			g.gain.setValueAtTime(snare ? 0.55 : 0.16, t);
			g.gain.exponentialRampToValueAtTime(0.001, t + len);
			src.connect(f);
			f.connect(g);
			g.connect(out);
			src.start(t, Math.random() * 0.5);
			src.stop(t + len + 0.02);
		}
		if (bar % 4 === 3 && s16 === 14 && I > 0.5) {
			// tom fill
			const o = ctx.createOscillator();
			const g = ctx.createGain();
			o.frequency.setValueAtTime(220, t);
			o.frequency.exponentialRampToValueAtTime(90, t + 0.2);
			g.gain.setValueAtTime(0.5, t);
			g.gain.exponentialRampToValueAtTime(0.001, t + 0.25);
			o.connect(g);
			g.connect(out);
			o.start(t);
			o.stop(t + 0.3);
		}
	}
}

