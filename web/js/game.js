/**
 * Murk game: rules, AI, input and UI. Ports the arcade logic of the original (MurkThink.cpp, Murk.cpp)
 * to a real-time loop with click-to-move, auto-fire, exploration fog and a minimap.
 */

// Artwork for the inline icon codes used by the original episode texts ("-p", "-k", ...).
const ICON_CODES = (() => {
  const map = {};
  'abcdefghi'.split('').forEach((c, i) => { map[c] = { sheet: 'p256', x: 384, y: i * 128 + 64, w: 64, h: 64, scale: 0.6 }; });
  const tiles = {
    j: [0, 294], k: [113, 294], l: [226, 294], m: [339, 294], n: [452, 294], o: [565, 294], p: [678, 294],
    q: [458, 0], r: [226, 490], s: [571, 0], t: [684, 0], u: [0, 294], v: [339, 490], w: [565, 490],
    x: [113, 0], y: [226, 0]
  };
  Object.keys(tiles).forEach((c) => { map[c] = { sheet: 'dungeon', x: tiles[c][0], y: tiles[c][1], w: 113, h: 98, scale: 0.5 }; });
  return map;
})();
const SHEET_SIZES = { p256: [800, 1280], dungeon: [1024, 1176] };

function escapeHtml(s) {
  return s.replace(/[&<>"]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
}

// Turns original episode text ("~" line breaks, "-x" icons) into HTML.
function richText(text) {
  const out = [];
  const src = text || '';
  for (let i = 0; i < src.length; i++) {
    const ch = src[i];
    if (ch === '~') {
      out.push('<br>');
    } else if (ch === '-' && ICON_CODES[src[i + 1]]) {
      const ic = ICON_CODES[src[i + 1]];
      const [sw, sh] = SHEET_SIZES[ic.sheet];
      const s = ic.scale;
      out.push(`<span class="icon" style="width:${Math.round(ic.w * s)}px;height:${Math.round(ic.h * s)}px;` +
        `background-image:url(assets/${ic.sheet === 'p256' ? 'p256' : 'dungeon'}.png);` +
        `background-size:${Math.round(sw * s)}px ${Math.round(sh * s)}px;` +
        `background-position:-${Math.round(ic.x * s)}px -${Math.round(ic.y * s)}px"></span>`);
      i++;
      while (src[i + 1] === ' ') i++;
    } else {
      out.push(escapeHtml(ch));
    }
  }
  return out.join('').replace(/\s{2,}/g, ' ');
}

class MurkGame {
  constructor() {
    this.container = document.getElementById('canvas-container');
    this.audio = new MurkAudio();
    this.ds = new DungeonSystem();
    this.renderer = null;

    this.state = 'LOADING';
    this.classKey = 'fighter';
    this.episodeNum = 1;
    this.lvl = 1;
    this.player = null;
    this.entities = [];
    this.projectiles = [];
    this.home = null;
    this.deaths = 0;

    this.held = [];
    this.keysDown = {};
    this.mouse = { x: 0, y: 0, left: false, right: false };
    this.path = [];
    this.autoUse = false;
    this.lastPathCell = null;
    this.transition = false;
    this.wakeTimer = 0;
    this.pitTimer = 4;
    this.minimapTimer = 0;
    this.hintCooldown = {};
    this.lastSoundAt = {};
    this.stepFlip = 0;
    this.time = 0;
    this.lastFrame = 0;

    this.init();
  }

  async init() {
    this.renderer = new MurkRenderer(this.container);
    try {
      const res = await fetch('assets/levels.json');
      this.ds.loadLevelDefs(await res.json());
    } catch (e) {
      console.warn('Could not load levels.json, using the built-in episode.', e);
    }
    await this.renderer.loadTextures();
    this.cacheDom();
    this.renderAvatars();
    this.bindEvents();
    this.state = 'SPLASH';
    document.getElementById('splash-screen').classList.add('ready');
    requestAnimationFrame((t) => this.gameLoop(t));
  }

  cacheDom() {
    const $ = (id) => document.getElementById(id);
    this.dom = {
      hud: $('hud'), log: $('log'), prompt: $('prompt'), fade: $('fade'), flash: $('flash'),
      hpFill: $('hp-fill'), hpText: $('hp-text'), arFill: $('ar-fill'), arText: $('ar-text'),
      skill: $('hud-skill'), level: $('hud-level'), rings: $('hud-rings'), floor: $('hud-floor'),
      episode: $('hud-episode'), relic: $('hud-relic'), portrait: $('hud-portrait'), minimap: $('minimap'),
      modal: $('modal-screen'), modalTitle: $('modal-title'), modalBody: $('modal-content'), modalBtn: $('modal-close'),
      btnRing: $('btn-ring'), btnMute: $('btn-mute')
    };
    this.mapCtx = this.dom.minimap.getContext('2d');
  }

  renderAvatars() {
    const avatars = { fighter: 0, cleric: 512, thief: 256, wizard: 384 };
    const img = new Image();
    img.src = 'assets/p256.png';
    img.onload = () => {
      this.sheetImg = img;
      Object.keys(avatars).forEach((key) => {
        const c = document.getElementById(`avatar-${key}`);
        if (!c) return;
        const ctx = c.getContext('2d');
        ctx.imageSmoothingEnabled = false;
        ctx.drawImage(img, 384, avatars[key] + 64, 64, 64, 0, 0, 64, 64);
      });
    };
  }

  // ---------- input ----------

  bindEvents() {
    const splash = document.getElementById('splash-screen');
    splash.addEventListener('click', () => {
      this.audio.init();
      this.audio.play('button.wav');
      splash.classList.add('hidden');
      document.getElementById('select-screen').classList.remove('hidden');
      this.state = 'SELECT';
    });

    const cards = document.querySelectorAll('.char-card');
    cards.forEach((card) => {
      card.addEventListener('click', () => {
        cards.forEach(c => c.classList.remove('selected'));
        card.classList.add('selected');
        this.classKey = card.getAttribute('data-class');
        this.audio.play('button.wav');
      });
    });

    document.getElementById('btn-select-confirm').addEventListener('click', () => {
      this.audio.play('Spawn.wav');
      document.getElementById('select-screen').classList.add('hidden');
      this.dom.hud.classList.remove('hidden');
      this.startNewGame();
    });

    this.dom.btnRing.addEventListener('click', () => this.useRing());
    document.getElementById('btn-drop').addEventListener('click', () => this.dropRelic(this.player, true));
    document.getElementById('btn-stairs').addEventListener('click', () => this.useStairs());
    document.getElementById('btn-help').addEventListener('click', () => this.showHelp());
    document.getElementById('btn-obj').addEventListener('click', () => this.showObjective());
    this.dom.btnMute.addEventListener('click', () => this.toggleMute());
    this.dom.modalBtn.addEventListener('click', () => this.closeModal());

    const surface = this.renderer.renderer.domElement;
    surface.addEventListener('mousedown', (e) => {
      if (this.state !== 'PLAYING') return;
      this.audio.init();
      this.mouse.x = e.clientX; this.mouse.y = e.clientY;
      if (e.button === 0) {
        this.mouse.left = true;
        this.lastPathCell = null;
        this.pathToMouse(true);
      } else if (e.button === 2) {
        this.mouse.right = true;
      }
    });
    window.addEventListener('mousemove', (e) => { this.mouse.x = e.clientX; this.mouse.y = e.clientY; });
    window.addEventListener('mouseup', (e) => {
      if (e.button === 0) this.mouse.left = false;
      if (e.button === 2) this.mouse.right = false;
    });
    surface.addEventListener('dblclick', () => {
      if (this.state === 'PLAYING') this.useStairs();
    });
    window.addEventListener('contextmenu', (e) => e.preventDefault());
    surface.addEventListener('wheel', (e) => {
      e.preventDefault();
      this.renderer.zoomBy(e.deltaY < 0 ? 1 : -1);
    }, { passive: false });
    window.addEventListener('blur', () => { this.held = []; this.keysDown = {}; this.mouse.left = this.mouse.right = false; });

    window.addEventListener('keydown', (e) => this.onKeyDown(e));
    window.addEventListener('keyup', (e) => this.onKeyUp(e));
  }

  moveKeyDir(e) {
    switch (e.code) {
      case 'ArrowLeft': case 'KeyA': case 'Numpad7': case 'Numpad4': return DIR.NW;
      case 'ArrowUp': case 'KeyW': case 'Numpad9': case 'Numpad8': return DIR.NE;
      case 'ArrowDown': case 'KeyS': case 'Numpad1': case 'Numpad2': return DIR.SW;
      case 'ArrowRight': case 'KeyD': case 'Numpad3': case 'Numpad6': return DIR.SE;
      default: return 0;
    }
  }

  onKeyDown(e) {
    if (e.repeat) {
      if (this.state === 'PLAYING' && (this.moveKeyDir(e) || e.code === 'Space' || e.code.startsWith('Control'))) e.preventDefault();
      return;
    }
    if (this.state === 'MODAL') {
      if (e.code === 'Escape' || e.code === 'Enter') {
        e.preventDefault();
        this.closeModal();
      }
      return;
    }
    if (this.state !== 'PLAYING') return;

    const dir = this.moveKeyDir(e);
    if (dir) {
      e.preventDefault();
      this.held = this.held.filter(d => d !== dir);
      this.held.push(dir);
      this.path = [];
      this.autoUse = false;
      return;
    }
    switch (e.code) {
      case 'Space': case 'ControlLeft': case 'ControlRight':
        e.preventDefault();
        this.keysDown.fire = true;
        break;
      case 'ShiftLeft': case 'ShiftRight': case 'KeyE': case 'Enter':
        e.preventDefault();
        this.useStairs();
        break;
      case 'KeyR': this.useRing(); break;
      case 'KeyT': this.dropRelic(this.player, true); break;
      case 'KeyM': this.toggleMute(); break;
      case 'F1': e.preventDefault(); this.showHelp(); break;
      case 'F2': e.preventDefault(); this.showObjective(); break;
      case 'KeyP': case 'Escape': this.showModal('PAUSED', 'The dungeon waits for you...', 'RESUME'); break;
      case 'Equal': case 'NumpadAdd': this.renderer.zoomBy(1); break;
      case 'Minus': case 'NumpadSubtract': this.renderer.zoomBy(-1); break;
      default: break;
    }
  }

  onKeyUp(e) {
    const dir = this.moveKeyDir(e);
    if (dir) this.held = this.held.filter(d => d !== dir);
    if (e.code === 'Space' || e.code.startsWith('Control')) this.keysDown.fire = false;
  }

  // ---------- game flow ----------

  startNewGame() {
    this.episodeNum = 1;
    this.player = null;
    this.beginEpisode();
  }

  beginEpisode() {
    const def = CLASS_DEFS[this.classKey];
    const ep = this.ds.getEpisode(def.file, this.episodeNum);
    this.ds.build(ep);
    this.projectiles = [];
    this.entities = [];
    this.path = [];
    this.renderer.clearEntities();
    this.deaths = this.deaths || 0;

    const home = this.ds.randomFloor(1);
    this.home = { x: home.x, y: home.y, lvl: 1 };
    this.ds.cell(home.x, home.y, 1).pad = def.pad;

    if (!this.player) {
      this.player = new Entity({
        kind: 'player', type: 'player', name: def.name, image: def.image, team: 1, classKey: this.classKey,
        x: home.x, y: home.y, lvl: 1, hp: def.hp + Math.floor(Math.random() * 10),
        armour: def.armour, rings: def.rings, dmin: def.dmin, dmax: def.dmax, shots: def.shots,
        fireCd: def.fireCd, tileTime: def.tileTime, fireSound: def.fireSound
      });
      this.player.level = 1;
      this.player.nextLevelAt = 25;
      this.player.perk = 0;
    } else {
      this.player.teleport(home.x, home.y, 1);
      this.player.hp = this.player.maxHp;
      this.player.protection = 0;
      this.player.carrying = 0;
      this.player.shots = 0;
      this.player.fireTimer = 0;
    }
    this.player.dir = DIR.SE;
    this.player.alive = true;
    this.player.deathTimer = 0;
    this.entities.push(this.player);

    this.spawnPopulation(ep);
    this.lvl = 1;
    this.dom.prompt.classList.remove('on');
    this.setupLevelView(true);
    this.updateHUD();
    this.pitTimer = 3;
    this.showObjective(true);
  }

  spawnPopulation(ep) {
    const ds = this.ds;
    const maxl = ds.maxlevel;
    const monsterTotal = ep.beholders + ep.deathorb + ep.skeletons + ep.mouther + ep.ogre;

    const lairs = [];
    for (let i = 0; i < Math.ceil(monsterTotal / 5); i++) {
      const lvl = Math.min(maxl - 1, this.randInt(maxl - 1) + 2);
      const p = ds.randomFloor(Math.max(2, lvl));
      if (p) {
        ds.cell(p.x, p.y, p.lvl).pad = 4;
        lairs.push({ x: p.x, y: p.y, lvl: p.lvl });
      }
    }

    const spawnMonsters = (key, count, speed) => {
      const def = MONSTER_DEFS[key];
      for (let i = 0; i < count; i++) {
        const lvl = this.randInt(maxl - 1) + 2;
        const p = ds.randomFloor(lvl, (x, y) => this.isFarFromHome(x, y, lvl));
        if (!p) continue;
        const e = new Entity({
          kind: 'monster', type: key, name: def.name, image: def.image, team: 0, x: p.x, y: p.y, lvl,
          hp: def.hp + Math.min(30, this.episodeNum - 1) + this.randInt(3), dmin: def.dmin, dmax: def.dmax, shots: def.shots, fireCd: def.fireCd,
          aggr: def.aggr, tileTime: BASE_TILE_TIME * (1 + 0.55 * (Math.max(1, speed) - 1)),
          asleep: true, lair: lairs.length ? lairs[this.entities.length % lairs.length] : null,
          fireSound: [2, 4, 5, 9].includes(def.image) ? 'fireball.wav' : 'throw.wav'
        });
        e.dir = 1 + this.randInt(4);
        if (!e.lair) e.lair = { x: p.x, y: p.y, lvl };
        this.entities.push(e);
      }
    };
    spawnMonsters('beholder', ep.beholders, ep.bspeed);
    spawnMonsters('mouther', ep.mouther, ep.mspeed);
    spawnMonsters('ogre', ep.ogre, ep.ospeed);
    spawnMonsters('skeleton', ep.skeletons, ep.sspeed);
    spawnMonsters('deathorb', ep.deathorb, ep.dspeed);

    const spawnCharacters = (key, count, team, speed) => {
      const def = CLASS_DEFS[key];
      for (let i = 0; i < count; i++) {
        const friendly = team === 1;
        const lvl = friendly ? 1 : maxl;
        const p = ds.randomFloor(lvl, (x, y) => !friendly || Math.abs(x - this.home.x) + Math.abs(y - this.home.y) > 2);
        if (!p) continue;
        const e = new Entity({
          kind: friendly ? 'ally' : 'rival', type: key, name: def.name, image: def.image, team: friendly ? 1 : 2,
          x: p.x, y: p.y, lvl, hp: def.hp + this.randInt(10) + this.episodeNum, armour: def.armour,
          dmin: def.dmin, dmax: def.dmax, shots: def.shots, fireCd: def.fireCd + 0.15, aggr: 0.75,
          tileTime: BASE_TILE_TIME * (1 + 0.55 * (Math.max(1, speed) - 1)) * 0.9, asleep: !friendly,
          fireSound: def.fireSound
        });
        e.lair = { x: p.x, y: p.y, lvl };
        this.entities.push(e);
      }
    };
    spawnCharacters('fighter', ep.fighter, ep.fteam, ep.fspeed);
    spawnCharacters('cleric', ep.cleric, ep.cteam, ep.cspeed);
    spawnCharacters('thief', ep.thief, ep.tteam, ep.tspeed);
    spawnCharacters('wizard', ep.wizard, ep.wteam, ep.wspeed);
  }

  isFarFromHome(x, y, lvl) {
    return lvl !== 1 || Math.abs(x - this.home.x) + Math.abs(y - this.home.y) > 6;
  }

  randInt(n) {
    return Math.floor(Math.random() * n);
  }

  setupLevelView(snap) {
    this.renderer.buildLevel(this.ds, this.lvl);
    this.ds.reveal(this.lvl, this.player.x, this.player.y);
    this.renderer.syncCells(this.ds, this.lvl);
    if (snap) this.renderer.camSnap = true;
    this.updateHUD();
  }

  // ---------- stairs and falling ----------

  useStairs() {
    const p = this.player;
    if (this.state !== 'PLAYING' || this.transition || !p.alive || p.moving) return;
    const cell = this.ds.cell(p.x, p.y, this.lvl);
    if (cell.type !== 's') {
      this.hint('stairs', 'There is no ladder here.');
      return;
    }
    if (cell.stair === 'down') {
      this.changeLevel(this.lvl + 1, p.x, p.y, 'You climb down to floor');
    } else if (this.lvl > 1) {
      this.changeLevel(this.lvl - 1, p.x, p.y, 'You climb up to floor');
    }
  }

  changeLevel(target, x, y, msg, fall = false) {
    if (this.transition) return;
    this.transition = true;
    this.path = [];
    this.autoUse = false;
    this.dom.fade.classList.add('on');
    this.audio.play(fall ? 'Protection.wav' : 'walk.wav', 0.5);
    setTimeout(() => {
      this.projectiles = [];
      this.player.shots = 0;
      this.player.teleport(x, y, target);
      this.entities.forEach((e) => { if (e !== this.player) e.shots = 0; });
      this.lvl = target;
      this.setupLevelView(true);
      this.onPlayerArrive();
      this.toast(`${msg} ${target}.`);
      this.dom.fade.classList.remove('on');
      setTimeout(() => { this.transition = false; }, 200);
    }, 260);
  }

  updatePits(dt) {
    const ds = this.ds;
    const ep = ds.currentEpisodeDef;
    if (ep && ep.pits) {
      this.pitTimer -= dt;
      const active = ds.pits.filter(p => p.active).length;
      if (this.pitTimer <= 0) {
        this.pitTimer = 3 + Math.random() * 4;
        if (active < Math.min(6, ep.maxpit || 6)) this.spawnPit();
      }
    }
    for (const pit of ds.pits) {
      if (!pit.active) continue;
      pit.age += dt;
      pit.frame = Math.floor(pit.age / 0.083) + 1;
      if (pit.frame >= 36) pit.active = false;
    }
    const p = this.player;
    if (!p.alive || p.moving || this.transition || p.lvl >= ds.maxlevel) return;
    for (const pit of ds.pits) {
      if (pit.active && pit.lvl === this.lvl && pit.x === p.x && pit.y === p.y && pit.frame >= 18 && pit.frame <= 29) {
        pit.active = false;
        this.toast('The floor gives way beneath you!', 'warn');
        this.renderer.addShake(8);
        this.changeLevel(this.lvl + 1, p.x, p.y, 'You fall to floor', true);
        return;
      }
    }
  }

  spawnPit() {
    const ds = this.ds;
    for (let tries = 0; tries < 60; tries++) {
      const lvl = this.randInt(ds.maxlevel - 1) + 1;
      const x = this.randInt(ds.N) + 1;
      const y = this.randInt(ds.N) + 1;
      if (!ds.isFreeFloor(x, y, lvl) || !ds.walkable(x, y, lvl + 1)) continue;
      if (ds.pits.some(p => p.active && p.x === x && p.y === y && p.lvl === lvl)) continue;
      let pit = ds.pits.find(p => !p.active);
      if (!pit) {
        pit = {};
        ds.pits.push(pit);
      }
      Object.assign(pit, { x, y, lvl, frame: 1, age: 0, active: true });
      return;
    }
  }

  // ---------- player actions ----------

  entityAt(x, y, lvl, ignore) {
    return this.entities.find(e => e !== ignore && e.alive && e.lvl === lvl && e.x === x && e.y === y) || null;
  }

  tryStep(e, dir) {
    e.dir = dir;
    const nx = e.x + DIR_VEC[dir][0];
    const ny = e.y + DIR_VEC[dir][1];
    if (!this.ds.walkable(nx, ny, e.lvl)) return false;
    if (this.entityAt(nx, ny, e.lvl, e)) return false;
    e.startMove(nx, ny, dir);
    return true;
  }

  updatePlayerControl() {
    const p = this.player;
    if (!p.alive || p.moving || this.transition) return;

    if (this.mouse.left) this.pathToMouse(false);

    let dir = this.held.length ? this.held[this.held.length - 1] : 0;
    if (!dir && this.path.length) {
      const next = this.path[0];
      const dx = next.x - p.x;
      const dy = next.y - p.y;
      if (Math.abs(dx) + Math.abs(dy) !== 1) {
        this.path = [];
      } else if (this.entityAt(next.x, next.y, this.lvl, p)) {
        p.dir = dirFromDelta(dx, dy);
        this.path = [];
        this.lastPathCell = null;
      } else {
        dir = dirFromDelta(dx, dy);
        this.path.shift();
      }
    }
    if (dir && this.tryStep(p, dir)) {
      if ((this.stepFlip++ & 1) === 0) this.audio.play('walk.wav', 0.25);
    } else if (!dir && this.autoUse) {
      this.autoUse = false;
      if (this.ds.cell(p.x, p.y, this.lvl).type === 's') this.useStairs();
    }
  }

  pathToMouse(force) {
    const g = this.renderer.screenToGrid(this.mouse.x, this.mouse.y);
    const key = `${g.x},${g.y}`;
    if (!force && key === this.lastPathCell) return;
    this.lastPathCell = key;
    const p = this.player;
    if (!this.ds.walkable(g.x, g.y, this.lvl)) return;
    if (g.x === p.x && g.y === p.y) {
      if (force) this.useStairs();
      return;
    }
    const path = this.ds.findPath(this.lvl, p.x, p.y, g.x, g.y, null);
    if (path) {
      this.path = path;
      this.autoUse = this.ds.cell(g.x, g.y, this.lvl).type === 's';
    }
  }

  updatePlayerFire() {
    const p = this.player;
    if (!p.alive) return;
    if (this.mouse.right) {
      const g = this.renderer.screenToGrid(this.mouse.x, this.mouse.y);
      const dir = dirFromDelta(g.fx - p.x, g.fy - p.y);
      this.fire(p, dir);
    } else if (this.keysDown.fire) {
      this.fire(p, p.dir);
    }
  }

  fire(e, dir) {
    if (!e.alive || e.fireTimer > 0 || e.shots >= e.maxShots) return false;
    e.dir = dir;
    e.fireTimer = e.fireCd;
    e.shots++;
    e.shotFlash = 0.1;
    const proj = new Projectile(e, dir);
    this.projectiles.push(proj);
    this.playAt(e.fireSound || 'throw.wav', e, 0.5);
    return true;
  }

  useRing() {
    const p = this.player;
    if (this.state !== 'PLAYING' || !p.alive) return;
    if (p.protection > 0) { this.hint('ring', 'The ring is already protecting you.'); return; }
    if (p.rings <= 0) { this.hint('ring', 'You have no Rings of Protection.'); return; }
    p.rings--;
    p.protection = 7;
    this.audio.play('Protection.wav', 0.6);
    this.audio.play('tcleric.wav', 0.25);
    this.toast('Ring of Protection! You are invulnerable.', 'good');
    this.updateHUD();
  }

  dropRelic(ent, voluntary) {
    if (!ent || !ent.carrying) {
      if (voluntary) this.hint('drop', 'You are not carrying a relic.');
      return;
    }
    const relic = this.ds.relic(ent.carrying);
    ent.carrying = 0;
    if (relic) {
      relic.carrier = null;
      relic.x = ent.x; relic.y = ent.y; relic.lvl = ent.lvl;
      relic.dropCooldown = voluntary ? 1.5 : 0.6;
    }
    if (voluntary) {
      this.toast('You set down the relic.');
      this.audio.play('button.wav');
    }
    this.updateHUD();
  }

  toggleMute() {
    const muted = this.audio.toggleMute();
    this.dom.btnMute.textContent = muted ? 'Sound: Off' : 'Sound: On';
  }

  // ---------- arrival, pickups, relics ----------

  onPlayerArrive() {
    const p = this.player;
    const ds = this.ds;
    if (ds.reveal(this.lvl, p.x, p.y)) this.renderer.syncCells(ds, this.lvl);

    const cell = ds.cell(p.x, p.y, this.lvl);
    switch (cell.item) {
      case 'e': {
        if (p.hp >= p.maxHp) { this.hint('elixir', 'You are already at full health.'); break; }
        const gain = Math.min(p.maxHp - p.hp, 6);
        p.hp += gain;
        this.popText(p, `+${gain}`, '#7dff9b');
        this.consumeItem(cell, 'Elixir restores your health.', 'item.wav');
        break;
      }
      case 'a':
        if (!CLASS_DEFS[this.classKey].wearsArmour) { this.hint('armour', 'This armour does not suit a spellcaster.'); break; }
        p.armour += 5; this.popText(p, '+5 armour', '#9fd8ff');
        this.consumeItem(cell, 'You don the armour (+5 armour).', 'item.wav');
        break;
      case 's':
        if (!CLASS_DEFS[this.classKey].readsScrolls) { this.hint('scroll', 'Only spellcasters can read this scroll.'); break; }
        p.armour += 5; this.popText(p, '+5 ward', '#9fd8ff');
        this.consumeItem(cell, 'The scroll weaves a protective ward (+5 armour).', 'item.wav');
        break;
      case 'r':
        p.rings++; this.popText(p, '+1 ring', '#ffe27a');
        this.consumeItem(cell, 'A Ring of Protection! (press R to use)', 'item.wav');
        break;
      default: break;
    }

    for (const relic of ds.relics) {
      if (relic.carrier || relic.lvl !== this.lvl || relic.x !== p.x || relic.y !== p.y) continue;
      if ((relic.dropCooldown || 0) > 0) continue;
      if (p.carrying) { this.hint('carry', 'You can only carry one relic at a time.'); continue; }
      relic.carrier = p;
      p.carrying = relic.id;
      this.audio.play('treasure.wav', 0.7);
      this.renderer.burst(this.renderer.gridToWorld(p.x, p.y).x, this.renderer.gridToWorld(p.x, p.y).y + 30, relic.id === 1 ? 0xff6655 : 0x66bbff, 18, 90, 0.7, 14);
      if (relic.id === 2) {
        this.toast('You found the relic! Carry it back to your pad - the dungeon awakens!', 'good');
        if (ds.currentEpisodeDef.gametype === 1) this.wakeAll();
      } else {
        this.toast('You took the red relic. Return it to your pad for a bonus.', 'good');
      }
      this.updateHUD();
    }

    if (p.carrying && this.home && p.lvl === this.home.lvl && p.x === this.home.x && p.y === this.home.y) {
      this.deliverRelic();
    }

    this.dom.prompt.classList.toggle('on', cell.type === 's');
    if (cell.type === 's') {
      this.dom.prompt.textContent = cell.stair === 'down' ? 'Press SHIFT / E or double-click to climb down' : 'Press SHIFT / E or double-click to climb up';
    }
    this.updateHUD();
  }

  consumeItem(cell, msg, sound) {
    cell.item = 0;
    this.toast(msg);
    this.audio.play(sound, 0.5);
    this.renderer.syncCells(this.ds, this.lvl);
    const w = this.renderer.gridToWorld(this.player.x, this.player.y);
    this.renderer.burst(w.x, w.y + 24, 0xfff0a0, 12, 70, 0.6, 10);
  }

  deliverRelic() {
    const p = this.player;
    const id = p.carrying;
    const relic = this.ds.relic(id);
    p.carrying = 0;
    if (relic) { relic.carrier = null; relic.lvl = 0; }
    if (id === 1) {
      this.addSkill(30);
      this.toast('Relic secured! +30 skill.', 'good');
      this.audio.play('win.wav', 0.6);
      this.updateHUD();
      return;
    }
    this.audio.play('win.wav', 0.8);
    p.armour += 30;
    p.rings += 3;
    this.addSkill(30);
    this.updateHUD();
    this.state = 'MODAL';
    this.modalAction = () => {
      this.episodeNum++;
      this.beginEpisode();
    };
    this.showModal(
      'EPISODE COMPLETE!',
      `You carried the relic home!<br><br>Reward: +30 skill, +30 armour, +3 Rings of Protection.<br><br>Descend to Episode ${this.episodeNum + 1}...`,
      'ONWARD', true
    );
  }

  wakeAll() {
    this.renderer.addShake(10);
    this.audio.play('DRAGON.WAV', 0.5);
    for (const e of this.entities) {
      if (e.asleep) e.asleep = false;
    }
  }

  // ---------- combat ----------

  hostile(a, b) {
    if (a === b || !a.alive || !b.alive) return false;
    if (a.isMonster) return !b.isMonster;
    if (b.isMonster) return true;
    return a.team !== b.team;
  }

  updateProjectiles(dt) {
    for (let i = this.projectiles.length - 1; i >= 0; i--) {
      const p = this.projectiles[i];
      if (p.lvl === this.lvl) {
        p.t += dt * p.speed;
        while (p.alive && p.t >= 1) {
          p.t -= 1;
          const nx = p.x + DIR_VEC[p.dir][0];
          const ny = p.y + DIR_VEC[p.dir][1];
          if (!this.ds.walkable(nx, ny, p.lvl)) {
            this.missileSpark(p);
            p.alive = false;
            break;
          }
          p.x = nx; p.y = ny;
          p.travelled++;
          if (p.travelled >= p.maxRange) { p.alive = false; break; }
          this.missileHit(p);
        }
        if (p.alive) this.missileHit(p);
        if (p.alive && Math.random() < dt * 40) {
          const rc = p.renderCell();
          const w = this.renderer.gridToWorld(rc.x, rc.y);
          const colors = { 1: 0xdddddd, 2: 0xff6644, 3: 0xffa030, 4: 0x66ccff, 5: 0x999999, 6: 0x77ff55 };
          this.renderer.spawnParticle(w.x + (Math.random() - 0.5) * 6, w.y + 30 + (Math.random() - 0.5) * 6, 0, 8, 0.35, 10, colors[p.kind] || 0xffffff, -6);
        }
      } else {
        p.alive = false;
      }
      if (!p.alive) {
        p.owner.shots = Math.max(0, p.owner.shots - 1);
        this.projectiles.splice(i, 1);
      }
    }
  }

  missileSpark(p) {
    const rc = p.renderCell();
    const w = this.renderer.gridToWorld(rc.x, rc.y);
    this.renderer.burst(w.x, w.y + 28, 0xffcc88, 6, 50, 0.3, 8);
  }

  missileHit(p) {
    for (const e of this.entities) {
      if (e.lvl !== p.lvl || e.x !== p.x || e.y !== p.y || !this.hostile(p.owner, e)) continue;
      p.alive = false;
      this.applyHit(p.owner, e, p.dir);
      return;
    }
  }

  applyHit(attacker, target, dir) {
    const w = this.renderer.gridToWorld(target.x, target.y);
    const dmg = attacker.rollDamage();
    const res = target.takeHit(dmg);

    if (res.blocked) {
      this.popText(target, 'BLOCKED', '#8ae8ff');
      this.audio.play('impact.wav', 0.3);
      this.renderer.burst(w.x, w.y + 34, 0x88ddff, 8, 70, 0.35, 9);
      return;
    }

    this.playAt('impact.wav', target, 0.5);
    this.renderer.burst(w.x, w.y + 34, 0xff5533, 10, 80, 0.45, 10);
    this.popText(target, `-${dmg}`, target === this.player ? '#ff6a5a' : '#ffffff');
    if (target.asleep) target.asleep = false;

    if (target.carrying) {
      this.dropRelic(target, false);
      if (target === this.player) this.toast('You dropped the relic!', 'warn');
    }
    if (target === this.player) {
      this.audio.play('ouch.wav', 0.7);
      this.renderer.addShake(6);
      this.dom.flash.classList.remove('on');
      void this.dom.flash.offsetWidth;
      this.dom.flash.classList.add('on');
      this.updateHUD();
    }

    if (target.hp <= 0) this.kill(target, attacker);
  }

  addSkill(n) {
    const p = this.player;
    p.skill += n;
    while (p.skill >= p.nextLevelAt) {
      p.level++;
      p.nextLevelAt += 40 + p.level * 10;
      p.maxHp += 20;
      p.hp = Math.min(p.maxHp, p.hp + 20);
      let perk = '';
      switch (p.perk) {
        case 0: p.maxShots = Math.min(6, p.maxShots + 1); perk = 'more missiles in flight'; break;
        case 1: p.dmax = Math.min(7, p.dmax + 1); perk = 'higher maximum damage'; break;
        case 2: p.fireCd = Math.max(0.14, p.fireCd * 0.88); perk = 'faster fire rate'; break;
        default: p.dmin = Math.min(p.dmax, p.dmin + 1); perk = 'higher minimum damage'; break;
      }
      p.perk = (p.perk + 1) % 4;
      this.toast(`LEVEL ${p.level}! +20 health and ${perk}.`, 'good');
      this.audio.play('win.wav', 0.5);
      const w = this.renderer.gridToWorld(p.x, p.y);
      this.renderer.burst(w.x, w.y + 30, 0xffe070, 26, 120, 0.9, 14);
    }
  }

  kill(target, killer) {
    target.alive = false;
    target.hp = 0;
    target.deathTimer = 0.8;
    const w = this.renderer.gridToWorld(target.x, target.y);
    this.renderer.burst(w.x, w.y + 30, 0xcc2222, 16, 90, 0.7, 12);
    if (target.carrying) this.dropRelic(target, false);

    if (target === this.player) {
      this.handlePlayerDeath();
      return;
    }
    this.playAt(target.isMonster ? 'die.wav' : 'dead.wav', target, 0.6);

    if (killer === this.player || (killer && killer.team === 1)) {
      const reward = Math.max(2, Math.round(target.maxHp / 3));
      if (killer === this.player) {
        this.popText(target, `+${reward}`, '#ffe27a');
        this.addSkill(reward);
        this.updateHUD();
      }
    }

    const cell = this.ds.cell(target.x, target.y, target.lvl);
    if (target.isMonster && cell.type === 'f' && cell.item === 0 && cell.pad < 0 && Math.random() < 0.7) {
      const r = this.randInt(3);
      cell.item = r === 0 ? 'a' : r === 1 ? 's' : 'e';
      if (target.lvl === this.lvl) this.renderer.syncCells(this.ds, this.lvl);
    }

    target.respawnTimer = target.isMonster ? 35 + Math.random() * 25 : 14;
  }

  handlePlayerDeath() {
    this.deaths++;
    this.audio.play('dead.wav', 0.8);
    this.path = [];
    this.held = [];
    this.keysDown.fire = false;
    this.mouse.right = false;
    this.updateHUD();
    const p = this.player;
    setTimeout(() => {
      this.modalAction = () => {
        this.player = null;
        this.beginEpisode();
      };
      this.showModal('YOU HAVE FALLEN',
        `The dungeon claims another soul.<br><br>Score: ${p.skill} &nbsp; Hero level: ${p.level}<br>Episode ${this.episodeNum}, floor ${this.lvl}<br><br>Rise again and try once more.`,
        'RISE AGAIN', true);
    }, 900);
  }

  respawnEntity(e) {
    const lair = e.lair || e.spawn;
    if (this.entityAt(lair.x, lair.y, lair.lvl, e)) return false;
    if (lair.lvl === this.lvl && Math.abs(lair.x - this.player.x) + Math.abs(lair.y - this.player.y) < 4) return false;
    e.hp = e.maxHp;
    e.alive = true;
    e.asleep = false;
    e.shots = 0;
    e.fireTimer = 0.5;
    e.teleport(lair.x, lair.y, lair.lvl);
    if (lair.lvl === this.lvl) {
      const w = this.renderer.gridToWorld(lair.x, lair.y);
      this.renderer.burst(w.x, w.y + 28, 0xaa44ff, 14, 70, 0.6, 12);
      this.playAt('Spawn.wav', e, 0.5);
    }
    return true;
  }

  // ---------- AI ----------

  wakeNearby() {
    const p = this.player;
    for (const e of this.entities) {
      if (!e.asleep || !e.alive || e.lvl !== this.lvl) continue;
      if (Math.abs(e.x - p.x) <= 3 && Math.abs(e.y - p.y) <= 3 && this.ds.findPath(this.lvl, e.x, e.y, p.x, p.y, null, 120)) {
        e.asleep = false;
        e.restTimer = 0.4;
        const def = MONSTER_DEFS[e.type];
        if (def) this.playAt(def.wake, e, 0.5);
      }
    }
  }

  findTarget(e) {
    for (const dir of DIRS) {
      const cells = this.ds.castLine(e.lvl, e.x, e.y, dir, 9);
      for (const c of cells) {
        const other = this.entityAt(c.x, c.y, e.lvl, e);
        if (!other) continue;
        if (this.hostile(e, other)) return { dir, other, dist: Math.abs(c.x - e.x) + Math.abs(c.y - e.y) };
        break;
      }
    }
    return null;
  }

  nearestHostile(e, range) {
    let best = null;
    let bestD = range + 1;
    for (const o of this.entities) {
      if (o.lvl !== e.lvl || !this.hostile(e, o)) continue;
      const d = Math.abs(o.x - e.x) + Math.abs(o.y - e.y);
      if (d < bestD) { bestD = d; best = o; }
    }
    return best;
  }

  think(e) {
    if (e.asleep) return;
    const shot = this.findTarget(e);
    if (shot && e.fireTimer <= 0 && e.shots < e.maxShots && e.restTimer <= 0) {
      e.dir = shot.dir;
      if (Math.random() < e.aggr) this.fire(e, shot.dir);
      e.restTimer = 0.12 + Math.random() * 0.25;
      return;
    }
    if (shot && e.restTimer > 0) return;

    let goal = null;
    if (e.kind === 'ally') {
      const foe = this.nearestHostile(e, 8);
      const d = Math.abs(this.player.x - e.x) + Math.abs(this.player.y - e.y);
      if (foe && d < 9) goal = foe;
      else if (d > 3 && this.player.lvl === e.lvl) goal = this.player;
    } else {
      goal = this.nearestHostile(e, 14);
    }

    if (goal) {
      const blocked = (x, y) => !!this.entityAt(x, y, e.lvl, e);
      const path = this.ds.findPath(e.lvl, e.x, e.y, goal.x, goal.y, blocked, 500);
      if (path && path.length > 1) {
        const step = path[0];
        if (this.tryStep(e, dirFromDelta(step.x - e.x, step.y - e.y))) return;
      } else if (path && path.length === 1) {
        e.dir = dirFromDelta(goal.x - e.x, goal.y - e.y);
        return;
      }
    }
    if (Math.random() < 0.55) {
      this.tryStep(e, 1 + this.randInt(4));
    } else {
      e.restTimer = 0.2 + Math.random() * 0.4;
    }
  }

  updateEntities(dt) {
    for (const e of this.entities) {
      if (!e.alive) {
        if (e.deathTimer > 0) e.deathTimer -= dt;
        if (e !== this.player) {
          e.respawnTimer -= dt;
          if (e.respawnTimer <= 0) {
            if (!this.respawnEntity(e)) e.respawnTimer = 2;
          }
        }
        continue;
      }
      const arrived = e.update(dt);
      if (e === this.player) {
        if (arrived) this.onPlayerArrive();
        for (const r of this.ds.relics) if (r.dropCooldown > 0) r.dropCooldown -= dt;
        continue;
      }
      if (e.lvl !== this.lvl || e.asleep) continue;
      e.thinkTimer -= dt;
      if (e.thinkTimer <= 0 && !e.moving) {
        e.thinkTimer = 0.06 + Math.random() * 0.08;
        this.think(e);
      }
    }
  }

  // ---------- UI ----------

  hint(key, msg) {
    const now = performance.now();
    if (now - (this.hintCooldown[key] || 0) < 2500) return;
    this.hintCooldown[key] = now;
    this.toast(msg);
  }

  toast(msg, kind = '') {
    const line = document.createElement('div');
    line.className = `log-line ${kind}`;
    line.textContent = msg;
    this.dom.log.appendChild(line);
    while (this.dom.log.children.length > 4) this.dom.log.removeChild(this.dom.log.firstChild);
    setTimeout(() => line.classList.add('fade'), 3800);
    setTimeout(() => line.remove(), 4600);
  }

  popText(ent, text, color) {
    const rc = ent.renderCell();
    const w = this.renderer.gridToWorld(rc.x, rc.y);
    this.renderer.floatText(w.x, w.y + 70, text, color);
  }

  playAt(name, ent, vol) {
    if (!this.player) return;
    const now = performance.now();
    if (now - (this.lastSoundAt[name] || 0) < 60) return;
    this.lastSoundAt[name] = now;
    if (ent.lvl !== this.lvl) return;
    const d = Math.abs(ent.x - this.player.x) + Math.abs(ent.y - this.player.y);
    if (d > 12) return;
    this.audio.play(name, vol * Math.max(0.2, 1 - d / 12));
  }

  updateHUD() {
    const p = this.player;
    if (!p) return;
    const d = this.dom;
    const hpPct = Math.max(0, Math.min(100, (p.hp / p.maxHp) * 100));
    d.hpFill.style.width = `${hpPct}%`;
    d.hpFill.classList.toggle('low', hpPct < 30);
    d.hpText.textContent = `${Math.max(0, Math.ceil(p.hp))} / ${p.maxHp}`;
    d.arFill.style.width = `${Math.min(100, p.armour * 2)}%`;
    d.arText.textContent = p.armour;
    d.skill.textContent = p.skill;
    d.level.textContent = p.level;
    d.rings.textContent = p.rings;
    d.floor.textContent = `${this.lvl} / ${this.ds.maxlevel}`;
    const ep = this.ds.currentEpisodeDef;
    d.episode.textContent = `Episode ${this.episodeNum}${ep && ep.title && ep.title !== 'Title' && !/^Episode/.test(ep.title) ? ` - ${ep.title}` : ''}`;
    d.btnRing.classList.toggle('active', p.protection > 0);
    d.relic.classList.toggle('on', !!p.carrying);
    d.relic.dataset.relic = p.carrying || '';
    this.drawPortrait();
  }

  drawPortrait() {
    if (!this.sheetImg || !this.player) return;
    const key = `${this.player.classKey}`;
    if (this.portraitKey === key) return;
    this.portraitKey = key;
    const rowY = { fighter: 0, cleric: 512, thief: 256, wizard: 384 }[key];
    const ctx = this.dom.portrait.getContext('2d');
    ctx.imageSmoothingEnabled = false;
    ctx.clearRect(0, 0, 64, 64);
    ctx.drawImage(this.sheetImg, 384, rowY + 64, 64, 64, 0, 0, 64, 64);
  }

  drawMinimap() {
    const ctx = this.mapCtx;
    const c = this.dom.minimap;
    ctx.clearRect(0, 0, c.width, c.height);
    const ds = this.ds;
    const p = this.player;
    const sx = 5;
    const sy = 2.5;
    const cx = c.width / 2;
    const cy = 14;
    const at = (x, y) => [cx + (x - y) * sx, cy + (x + y) * sy];

    ds.forEachCell(this.lvl, (cell, x, y) => {
      if (!cell.explored || (cell.type !== 'f' && cell.type !== 's')) return;
      const [px, py] = at(x, y);
      ctx.fillStyle = cell.type === 's' ? '#e0a040' : '#6a5a48';
      ctx.beginPath();
      ctx.moveTo(px, py - sy); ctx.lineTo(px + sx, py); ctx.lineTo(px, py + sy); ctx.lineTo(px - sx, py);
      ctx.closePath();
      ctx.fill();
      if (cell.pad === 0 || cell.pad === 1 || cell.pad === 2 || cell.pad === 3) { ctx.fillStyle = '#4fe08a'; ctx.fillRect(px - 1.5, py - 1.5, 3, 3); }
    });
    for (const r of ds.relics) {
      if (r.lvl === this.lvl && !r.carrier && ds.cell(r.x, r.y, this.lvl).explored) {
        const [px, py] = at(r.x, r.y);
        ctx.fillStyle = r.id === 1 ? '#ff5544' : '#55aaff';
        ctx.fillRect(px - 2, py - 2, 4, 4);
      }
    }
    for (const e of this.entities) {
      if (e === p || !e.alive || e.lvl !== this.lvl) continue;
      if (this.renderer.entityVisibility(ds, e, p) <= 0) continue;
      const [px, py] = at(e.x, e.y);
      ctx.fillStyle = e.kind === 'ally' ? '#66d0ff' : '#ff4a3a';
      ctx.fillRect(px - 1.5, py - 1.5, 3, 3);
    }
    const [ppx, ppy] = at(p.x, p.y);
    ctx.fillStyle = '#fff36a';
    ctx.beginPath();
    ctx.arc(ppx, ppy, 2.6, 0, Math.PI * 2);
    ctx.fill();
  }

  showObjective(first = false) {
    const ep = this.ds.currentEpisodeDef;
    const title = ep && ep.title && ep.title !== 'Title' ? ep.title : `Episode ${this.episodeNum}`;
    const body = richText(ep ? ep.response : 'Find the relic and return it home.');
    const hint = '<div class="modal-hint">Brave the lower floors, take the relic and carry it back to your glowing home pad.</div>';
    this.modalAction = null;
    this.showModal(title.toUpperCase(), body + hint, first ? 'ENTER THE DUNGEON' : 'CONTINUE', true);
  }

  showHelp() {
    this.modalAction = null;
    this.showModal('MURK INSTRUCTIONS', `
<b>Move</b> &nbsp; Left-click a tile (hold to follow the cursor), or Arrow keys / WASD<br>
<b>Fire</b> &nbsp; Right-click toward a diagonal, or Space / Ctrl (hold to keep firing)<br>
<b>Ladders</b> &nbsp; Stand on a ladder and press Shift / E, or double-click<br>
<b>Ring of Protection</b> &nbsp; R &nbsp; (invulnerable for a few seconds)<br>
<b>Set down relic</b> &nbsp; T &nbsp; | &nbsp; <b>Zoom</b> mouse wheel or +/- &nbsp; | &nbsp; <b>Mute</b> M<br>
<b>Pause</b> P &nbsp; | &nbsp; <b>Help</b> F1 &nbsp; | &nbsp; <b>Goal</b> F2<br><br>
Missiles fly along the four diagonals of the map - line up your shots. Taking a hit while holding the relic makes you drop it, so use your ring.
Monsters sleep until you come near; taking the relic wakes the whole dungeon. Collect elixirs (health), armour or scrolls (class dependent) and rings,
and level up by slaying monsters.`, 'CONTINUE', true);
  }

  showModal(title, html, buttonText = 'CONTINUE') {
    this.dom.modalTitle.textContent = title;
    this.dom.modalBody.innerHTML = html;
    this.dom.modalBtn.textContent = buttonText;
    this.dom.modal.classList.remove('hidden');
    this.state = 'MODAL';
    this.held = [];
    this.keysDown.fire = false;
    this.mouse.left = this.mouse.right = false;
  }

  closeModal() {
    this.dom.modal.classList.add('hidden');
    this.state = 'PLAYING';
    const action = this.modalAction;
    this.modalAction = null;
    this.audio.play('button.wav', 0.4);
    if (action) action();
  }

  // ---------- main loop ----------

  update(dt) {
    this.time += dt;
    this.wakeTimer -= dt;
    if (this.wakeTimer <= 0) {
      this.wakeTimer = 0.25;
      this.wakeNearby();
    }

    this.updatePlayerControl();
    this.updatePlayerFire();
    this.updateEntities(dt);
    this.updateProjectiles(dt);
    this.updatePits(dt);

    this.minimapTimer -= dt;
    if (this.minimapTimer <= 0) {
      this.minimapTimer = 0.1;
      this.drawMinimap();
      this.dom.btnRing.classList.toggle('active', this.player.protection > 0);
      if (this.player.protection > 0) this.dom.btnRing.dataset.left = Math.ceil(this.player.protection);
    }
  }

  gameLoop(time) {
    requestAnimationFrame((t) => this.gameLoop(t));
    const dt = Math.min(0.05, (time - this.lastFrame) / 1000) || 0.016;
    this.lastFrame = time;

    if (this.state === 'PLAYING') this.update(dt);

    if (this.player && this.ds.levels.length && (this.state === 'PLAYING' || this.state === 'MODAL')) {
      this.renderer.frame({
        ds: this.ds, lvl: this.lvl, entities: this.entities, projectiles: this.projectiles, player: this.player
      }, this.state === 'PLAYING' ? dt : 0);
    }
  }
}

window.addEventListener('DOMContentLoaded', () => {
  window.murkGame = new MurkGame();
});
