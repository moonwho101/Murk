import { HEROES, HERO_ORDER, TEXT_TOKENS } from './config.js';
import { T } from './dungeon.js';

const base = import.meta.env.BASE_URL + 'assets/';
const el = (id) => document.getElementById(id);

const TOKEN_ICONS = {
	a: { hero: 'fighter' }, b: { hero: 'beholder' }, c: { hero: 'thief' }, d: { hero: 'wizard' }, e: { hero: 'cleric' },
	g: { hero: 'skeleton' }, h: { hero: 'mouther' }, i: { hero: 'ogre' },
	j: { item: 'chestRed' }, k: { pad: '#4fe08a' }, l: { pad: '#ff5a3a' }, m: { pad: '#5a8aff' }, n: { pad: '#c58aff' },
	o: { pad: '#f0f0ff' }, p: { item: 'chestBlue' }, u: { item: 'chestRed' }, q: { item: 'armour' }, s: { item: 'scroll' },
	t: { item: 'ring' }, x: { item: 'stairsDown' }, y: { item: 'stairsUp' },
};

const EPISODE_NAMES = [
	'The Descent', 'Into the Murk', 'Shadows Stir', 'Depths Unknown', 'A Hollow Welcome', 'The Long Dark',
	'Whispers Below', 'Bones and Embers', 'The Gathering Gloom', 'Beneath the Stones', 'Cold Hearth, Warm Blood',
	'The Sunless Road',
];

export class UI {
	constructor(assets) {
		this.assets = assets;
		this.screen = el('screen');
		this.hud = el('hud');
		this.msgs = el('hud-msgs');
		this.floaters = [];
		this.floatLayer = el('floaters');
		this.minimap = el('minimap');
		this.mctx = this.minimap.getContext('2d');
		this.mapTimer = 0;
		this.msgCount = 0;
		this.view = null;
		this.hintTimer = 0;
	}

	// ----------------------------------------------------------------- icons
	itemIconStyle(name, size = 28) {
		const meta = this.assets.itemsMeta;
		const it = meta.items[name];
		const k = size / meta.cellH;
		const bb = it.bbox;
		// crop around the opaque pixels so icons are centred
		const cx = (bb[0] + bb[2]) / 2;
		const cy = (bb[1] + bb[3]) / 2;
		const sheetW = this.assets.items.image.width * k;
		const sheetH = this.assets.items.image.height * k;
		const x = -(it.index * meta.cellW + cx) * k + size / 2;
		const y = -cy * k + size / 2;
		return `width:${size}px;height:${size}px;background-image:url(${base}sprites/items.png);background-size:${sheetW}px ${sheetH}px;background-position:${x}px ${y}px;`;
	}

	heroIconStyle(block, size = 38) {
		const meta = this.assets.heroesMeta;
		const row = meta.blocks[block].row + 1;
		const k = size / 128;
		return `width:${size}px;height:${size}px;background-image:url(${base}sprites/heroes.png);background-size:${1600 * k}px ${2560 * k}px;background-position:${0}px ${-row * 128 * k}px;`;
	}

	tokenHtml(code) {
		const t = TOKEN_ICONS[code];
		const word = TEXT_TOKENS[code] || (TOKEN_ICONS[code]?.pad ? 'Rune Circle' : TOKEN_ICONS[code]?.item === 'chestRed' ? 'Red Chest' : '');
		if (!t) return word;
		let style;
		if (t.hero) style = this.heroIconStyle(t.hero, 40);
		else if (t.item) style = this.itemIconStyle(t.item, 36);
		else style = `width:30px;height:30px;border-radius:50%;border:3px solid ${t.pad};box-shadow:0 0 10px ${t.pad};background:radial-gradient(${t.pad}99,transparent 70%);`;
		return `<span class="tok" style="${style}" title="${word}"></span><em>${word}</em>`;
	}

	/** Convert an original episode text ("Find the -p   and return it to -k   .~~...") to HTML lines. */
	objectiveLines(text) {
		return text
			.split('~~')
			.map((seg) => seg.replace(/-([a-z])\s*/g, (_, c) => ` ${this.tokenHtml(c)} `).replace(/\s+([.,])/g, '$1').trim())
			.filter(Boolean);
	}

	// --------------------------------------------------------------- loading
	setLoading(f) {
		el('load-fill').style.width = Math.round(f * 100) + '%';
	}
	hideLoading() {
		el('loading').classList.add('done');
		setTimeout(() => el('loading').remove(), 800);
	}

	// ---------------------------------------------------------------- screens
	clearScreen() {
		this.screen.classList.add('hidden');
		this.screen.innerHTML = '';
		this.screen.classList.remove('solid');
	}
	open(html, { solid = false } = {}) {
		this.screen.innerHTML = html;
		this.screen.classList.remove('hidden');
		this.screen.classList.toggle('solid', solid);
		return this.screen;
	}

	showTitle({ hasSave, onNew, onContinue, onHelp, muted, onMute }) {
		this.hud.classList.add('hidden');
		const s = this.open(
			`<div class="title-screen" style="background-image:url(${base}ui/splash.png)">
				<div class="title-menu">
					${hasSave ? '<button class="btn" id="b-cont">Continue</button>' : ''}
					<button class="btn ${hasSave ? 'secondary' : ''}" id="b-new">New Game</button>
					<button class="btn secondary" id="b-help">How to Play</button>
					<button class="btn secondary small" id="b-mute">${muted ? 'Sound: Off' : 'Sound: On'}</button>
				</div>
				<div class="title-credit">MURK © 1998–2000 Calamity Software · Programmed by Mark Longo · Graphics by Tom Portaccio · three.js edition</div>
			</div>`,
			{ solid: true },
		);
		s.querySelector('#b-new').onclick = onNew;
		if (hasSave) s.querySelector('#b-cont').onclick = onContinue;
		s.querySelector('#b-help').onclick = onHelp;
		s.querySelector('#b-mute').onclick = (e) => {
			const m = onMute();
			e.target.textContent = m ? 'Sound: Off' : 'Sound: On';
		};
	}

	showSelect({ onPick, onBack }) {
		const cards = HERO_ORDER.map((id) => {
			const h = HEROES[id];
			const stats = Object.entries(h.stats)
				.map(([k, v]) => `<div class="statline"><span>${k}</span><i style="--v:${v * 20}%"></i></div>`)
				.join('');
			return `<div class="hero-card" data-hero="${id}">
				<img src="${base}ui/portrait_${id}.png" alt="${h.name}" />
				<h3>${h.name}</h3><div class="sub">${h.title}</div>
				<p>${h.blurb}</p>${stats}
			</div>`;
		}).join('');
		const s = this.open(
			`<div class="select-screen"><h2>Choose thy Character</h2><div class="hero-cards">${cards}</div>
			<button class="btn secondary small" id="b-back">Back</button></div>`,
			{ solid: true },
		);
		s.querySelectorAll('.hero-card').forEach((c) => (c.onclick = () => onPick(c.dataset.hero)));
		s.querySelector('#b-back').onclick = onBack;
	}

	showBriefing({ hero, episodeNo, episode, onBegin, retry = false }) {
		const lines = this.objectiveLines(episode.text);
		const h = HEROES[hero];
		const title = retry ? 'Try Again' : EPISODE_NAMES[(episodeNo - 1) % EPISODE_NAMES.length];
		const extra = [];
		if (episode.gametype === 2) extra.push('Rival adventurers are after your Red Chest — guard it, and slay them if you can.');
		if (episode.pits) extra.push('Beware the pits that yawn open without warning.');
		const s = this.open(
			`<div class="scroll-screen"><div class="scroll" style="background-image:url(${base}ui/scroll.png)">
				<div class="ep">Episode ${episodeNo}</div>
				<h2>${title}</h2>
				${lines.map((l) => `<div class="line">${l}</div>`).join('')}
				${extra.map((t) => `<div class="tip">${t}</div>`).join('')}
				<button class="btn" id="b-begin">Begin</button>
			</div></div>`,
			{ solid: false },
		);
		s.querySelector('#b-begin').onclick = onBegin;
	}

	showPause({ onResume, onRestart, onQuit, onHelp, audio, onVolume }) {
		const s = this.open(
			`<div class="dialog"><h2>Paused</h2>
			<div class="slider-row"><span>Sound effects</span><input type="range" id="v-sfx" min="0" max="100" value="${Math.round(audio.sfxVol * 100)}"></div>
			<div class="slider-row"><span>Music</span><input type="range" id="v-mus" min="0" max="100" value="${Math.round(audio.musicVol * 100)}"></div>
			<div class="btns">
				<button class="btn" id="b-resume">Resume</button>
				<button class="btn secondary" id="b-help">How to Play</button>
				<button class="btn secondary" id="b-restart">Restart Episode</button>
				<button class="btn secondary" id="b-quit">Quit to Title</button>
			</div></div>`,
		);
		const vol = () => onVolume(el('v-sfx').value / 100, el('v-mus').value / 100);
		s.querySelector('#v-sfx').oninput = vol;
		s.querySelector('#v-mus').oninput = vol;
		s.querySelector('#b-resume').onclick = onResume;
		s.querySelector('#b-help').onclick = onHelp;
		s.querySelector('#b-restart').onclick = onRestart;
		s.querySelector('#b-quit').onclick = onQuit;
	}

	showHelp({ onClose }) {
		const s = this.open(
			`<div class="dialog"><h2>How to Play</h2>
			<div class="help-grid">
				<kbd>W A S D</kbd><span>Move (arrow keys work too) — directions are relative to the screen</span>
				<kbd>Mouse</kbd><span>Aim. <b>Hold left click</b> to run toward the cursor; you stop when you release it or hit a wall. <kbd>Ctrl</kbd> fires your weapon</span>
				<kbd>Right click</kbd><span>Fire your weapon at the cursor without moving (hold to keep firing)</span>
				<kbd>R</kbd><span>Use a Ring: a few seconds of invulnerability; touching monsters destroys them</span>
				<kbd>E</kbd><span>Take stairs (standing on them for a moment also works)</span>
				<kbd>T</kbd><span>Drop the treasure you are carrying</span>
				<kbd>Wheel</kbd><span>Zoom the camera</span>
				<kbd>F3 / F4</kbd><span>Save / load your hero</span>
				<kbd>Esc</kbd><span>Pause menu</span>
			</div>
			<div class="help-note">Explore the dungeon, find the glowing <b>Blue Chest</b> and carry it back to the rune circle where you began. Taking it wakes everything — run! Elixirs heal, armour and scrolls shield you, rings protect you. Slay monsters to gain levels; they return after a while, so keep moving.</div>
			<div class="btns"><button class="btn" id="b-close">Close</button></div></div>`,
		);
		s.querySelector('#b-close').onclick = onClose;
	}

	showResult({ won, world, onNext, onRetry, onQuit, hero }) {
		const st = world.stats;
		const t = Math.round(world.time);
		const mm = String(Math.floor(t / 60)).padStart(2, '0');
		const ss = String(t % 60).padStart(2, '0');
		const p = world.player;
		const s = this.open(
			`<div class="dialog"><h2 class="${won ? '' : 'bad'}">${won ? 'Treasure Recovered!' : 'Your Treasure Was Stolen'}</h2>
			<div class="help-note" style="text-align:center">${won ? `${HEROES[hero].name} returns in triumph.` : 'Rivals carried the Red Chest to their lair.'}</div>
			<div class="stats">
				<div>Time<b>${mm}:${ss}</b></div><div>Level<b>${p.lvl}</b></div>
				<div>Monsters slain<b>${st.kills}</b></div><div>Deaths<b>${st.deaths}</b></div>
				<div>Items found<b>${st.items}</b></div><div>Experience<b>${p.xp}</b></div>
			</div>
			<div class="btns row">
				${won ? '<button class="btn" id="b-next">Next Episode</button>' : '<button class="btn" id="b-retry">Try Again</button>'}
				<button class="btn secondary" id="b-quit">Quit to Title</button>
			</div></div>`,
		);
		if (won) s.querySelector('#b-next').onclick = onNext;
		else s.querySelector('#b-retry').onclick = onRetry;
		s.querySelector('#b-quit').onclick = onQuit;
	}

	// -------------------------------------------------------------------- HUD
	showHud(hero) {
		this.hud.classList.remove('hidden');
		const h = HEROES[hero];
		el('hud-portrait').src = `${base}ui/portrait_${hero}.png`;
		el('hud-name').textContent = h.name;
		const ring = this.itemIconStyle('ring', 26);
		document.querySelector('.ico-ring').style.cssText = ring;
		document.querySelector('.ico-chest').style.cssText = this.itemIconStyle('chestBlue', 26);
		document.querySelector('.ico-skull').style.cssText = this.heroIconStyle('skeleton', 28);
		this.msgs.innerHTML = '';
		el('hud-hint').classList.remove('faded');
		this.hintTimer = 14;
		el('death-banner').classList.add('hidden');
	}

	say(text, duration = 4, kind = 'info') {
		const m = document.createElement('div');
		m.className = 'msg ' + kind;
		m.textContent = text;
		this.msgs.appendChild(m);
		while (this.msgs.children.length > 4) this.msgs.firstChild.remove();
		setTimeout(() => m.classList.add('out'), duration * 1000);
		setTimeout(() => m.remove(), duration * 1000 + 700);
	}

	setEpisode(world) {
		el('hud-episode').textContent = `Episode ${world.episodeNo}`;
		const lines = this.objectiveLines(world.def.text);
		el('hud-objective').innerHTML = lines[0] || '';
	}

	float(x, y, z, text, cls = 'dmg') {
		const d = document.createElement('div');
		d.className = 'floater ' + cls;
		d.textContent = text;
		this.floatLayer.appendChild(d);
		this.floaters.push({ d, x, y, z, t: 0, life: 1.1, dx: (Math.random() - 0.5) * 30 });
	}

	updateHud(world, view, dt) {
		const p = world.player;
		const f = (id, frac) => (el(id).style.width = Math.max(0, Math.min(100, frac * 100)) + '%');
		f('bar-hp', p.hp / (p.maxHp + 10));
		el('txt-hp').textContent = `${Math.max(0, Math.ceil(p.hp))} / ${p.maxHp}`;
		f('bar-armour', Math.min(1, p.armour / 40));
		el('txt-armour').textContent = Math.round(p.armour);
		f('bar-xp', (p.xp - (p.levelXp || 0)) / Math.max(1, p.nextXp - (p.levelXp || 0)));
		el('txt-xp').textContent = `${p.xp} / ${p.nextXp}`;
		el('hud-level').textContent = `Level ${p.lvl} · ${HEROES[p.type].title}`;
		el('txt-rings').textContent = p.rings;
		el('chip-ring').classList.toggle('ready', p.rings > 0 && p.protT <= 0);
		el('txt-kills').textContent = world.stats.kills;
		const carrying = p.carrying;
		el('txt-chest').textContent = carrying ? 'Carried!' : world.chestById('B')?.carrier ? '—' : 'Hunt';
		el('chip-treasure').classList.toggle('ready', !!carrying);
		el('hud-depth').textContent = `Depth ${p.level + 1} / ${world.levelCount}`;

		el('death-banner').classList.toggle('hidden', p.alive);

		// stairs hint
		const tt = world.tile(p.level, Math.round(p.x), Math.round(p.y));
		const sh = el('stairs-hint');
		if (p.alive && (tt === T.DOWN || tt === T.UP)) {
			sh.classList.remove('hidden');
			sh.innerHTML = tt === T.DOWN ? '<kbd>E</kbd> Descend the stairs ▼' : '<kbd>E</kbd> Climb the stairs ▲';
		} else sh.classList.add('hidden');

		// objective line
		const obj = el('hud-objective');
		if (carrying) obj.innerHTML = 'Return to the rune circle where you began!';
		else if (world.def.gametype === 2 && world.chestById('A')?.carrier) obj.innerHTML = 'Rivals have your Red Chest — stop them!';
		else obj.innerHTML = this.objectiveLines(world.def.text)[0] || '';

		// fade controls hint
		if (this.hintTimer > 0) {
			this.hintTimer -= dt;
			if (this.hintTimer <= 0) el('hud-hint').classList.add('faded');
		}

		this.updatePointer(world, view);

		// floaters
		for (let i = this.floaters.length - 1; i >= 0; i--) {
			const fl = this.floaters[i];
			fl.t += dt;
			if (fl.t >= fl.life) {
				fl.d.remove();
				this.floaters.splice(i, 1);
				continue;
			}
			const pt = view.project(fl.x, fl.y + fl.t * 0.9, fl.z);
			fl.d.style.left = pt.x + fl.dx * fl.t + 'px';
			fl.d.style.top = pt.y - 18 + 'px';
			fl.d.style.opacity = String(1 - Math.pow(fl.t / fl.life, 2));
		}

		this.mapTimer -= dt;
		if (this.mapTimer <= 0) {
			this.mapTimer = 0.1;
			this.drawMinimap(world);
		}
	}

	updatePointer(world, view) {
		const ptr = el('hud-pointer');
		const p = world.player;
		let goal = null;
		let label = '';
		const chest = world.chestById('B');
		if (p.carrying === 'B') {
			const pad = world.pads[0];
			goal = { level: pad.level, x: pad.x, y: pad.y };
			label = 'Home';
		} else if (chest && !chest.carrier && world.explored[chest.level][world.idx(Math.round(chest.x), Math.round(chest.y))]) {
			goal = { level: chest.level, x: Math.round(chest.x), y: Math.round(chest.y) };
			label = 'Blue Chest';
		}
		if (!goal || !p.alive) {
			ptr.style.opacity = '0';
			return;
		}
		const field = world.getField('ptr', goal);
		const r = world.routeStep(p, field);
		const dist = field[p.level][Math.round(p.y) * world.dungeon.w + Math.round(p.x)];
		if (!r || r.arrived || dist < 3) {
			ptr.style.opacity = '0';
			return;
		}
		let gx = r.vx;
		let gy = r.vy;
		if (r.stairs) {
			gx = 0;
			gy = 0;
		}
		const S = Math.SQRT1_2;
		const right = (gx - gy) * S;
		const up = (-gx - gy) * S;
		const ang = Math.atan2(right, up);
		const c = view.project(p.x, 0.5, p.y);
		const rad = 110;
		ptr.style.left = c.x + Math.sin(ang) * rad + 'px';
		ptr.style.top = c.y - Math.cos(ang) * rad + 'px';
		ptr.style.marginLeft = '-27px';
		ptr.style.marginTop = '-27px';
		ptr.style.right = 'auto';
		ptr.querySelector('svg').style.transform = `rotate(${ang}rad)`;
		el('pointer-label').textContent = `${label} · ${goal.level !== p.level ? 'Depth ' + (goal.level + 1) : dist + ' steps'}`;
		ptr.style.opacity = '0.9';
	}

	drawMinimap(world) {
		const g = this.mctx;
		const W = this.minimap.width;
		const { w, h } = world.dungeon;
		const p = world.player;
		const L = p.level;
		g.clearRect(0, 0, W, W);
		g.fillStyle = '#05060a';
		g.fillRect(0, 0, W, W);
		const cell = Math.floor((W - 8) / Math.max(w, h));
		const ox = Math.floor((W - cell * w) / 2);
		const oy = Math.floor((W - cell * h) / 2);
		const tiles = world.dungeon.levels[L].tiles;
		const ex = world.explored[L];
		for (let y = 0; y < h; y++) {
			for (let x = 0; x < w; x++) {
				const i = y * w + x;
				if (!ex[i] || tiles[i] === T.VOID) continue;
				const vis = world.visible[i] >= 0;
				g.fillStyle = tiles[i] === T.DOWN ? '#e8902c' : tiles[i] === T.UP ? '#5ab4f0' : vis ? '#8a7a5a' : '#4a4232';
				g.fillRect(ox + x * cell, oy + y * cell, cell - 1, cell - 1);
			}
		}
		const dot = (x, y, color, r = 1.5) => {
			g.fillStyle = color;
			g.beginPath();
			g.arc(ox + (x + 0.5) * cell, oy + (y + 0.5) * cell, (cell / 2) * r, 0, Math.PI * 2);
			g.fill();
		};
		for (const pad of world.pads) {
			if (pad.level === L && ex[pad.y * w + pad.x]) dot(pad.x, pad.y, pad.team === 1 ? '#6dff9a' : '#ff5a5a', 1.1);
		}
		for (const c of world.chests) {
			if (c.level === L && !c.carrier && ex[Math.round(c.y) * w + Math.round(c.x)]) dot(c.x, c.y, c.color === 'red' ? '#ff4040' : '#5a8aff', 1.3);
		}
		for (const [key, it] of world.items[L]) {
			if (ex[key]) dot(key % w, Math.floor(key / w), it.type === 'ring' ? '#ffe050' : '#c8a0ff', 0.6);
		}
		for (const e of world.entities) {
			if (!e.alive || e.level !== L || e.isPlayer) continue;
			if (world.visible[Math.round(e.y) * w + Math.round(e.x)] < 0) continue;
			dot(e.x, e.y, e.kind === 'monster' ? '#ff4a3a' : e.team === 1 ? '#6ab4ff' : '#ff8a8a', 1);
		}
		if (p.alive) {
			const blink = Math.floor(performance.now() / 300) % 2 === 0;
			dot(p.x, p.y, blink ? '#ffffff' : '#6dff9a', 1.5);
		}
	}
}
