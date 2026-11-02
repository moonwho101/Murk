/**
 * Murk Game Engine
 * Coordinates game states, player loop, monster AI behaviors, input, audio, and UI.
 */

class MurkGame {
  constructor() {
    this.container = document.getElementById('canvas-container');
    this.renderer = null;
    this.audio = new MurkAudio();
    this.dungeonSys = new DungeonSystem();

    this.player = null;
    this.monsters = [];
    this.projectiles = [];

    this.currentLevel = 1;
    this.currentEpisode = 1;
    this.selectedClass = 'fighter';
    this.classFile = 'levelsb.dat';

    this.gameState = 'SPLASH';
    this.lastTime = 0;
    this.monsterAITimer = 0;
    this.pitTrapTimer = 0;

    this.keys = {};
    this.init();
  }

  async init() {
    this.renderer = new MurkRenderer(this.container);

    try {
      const res = await fetch('assets/levels.json');
      const levelsData = await res.json();
      this.dungeonSys.loadLevelDefs(levelsData);
    } catch (e) {
      console.warn('Could not load levels.json, using defaults.');
    }

    await this.renderer.loadTextures();
    this.renderAvatars();
    this.bindEvents();

    requestAnimationFrame((t) => this.gameLoop(t));
  }

  renderAvatars() {
    const avatars = [
      { id: 'avatar-fighter', y: 0 },
      { id: 'avatar-cleric', y: 512 },
      { id: 'avatar-thief', y: 256 },
      { id: 'avatar-wizard', y: 384 }
    ];

    const img = new Image();
    img.src = 'assets/p256.png';
    img.onload = () => {
      avatars.forEach(av => {
        const c = document.getElementById(av.id);
        if (!c) return;
        const ctx = c.getContext('2d');
        ctx.imageSmoothingEnabled = false;
        ctx.drawImage(img, 384, av.y + 64, 64, 64, 0, 0, 64, 64);
      });
    };
  }

  bindEvents() {
    const splash = document.getElementById('splash-screen');
    splash.addEventListener('click', () => {
      this.audio.init();
      this.audio.play('button.wav');
      splash.classList.add('hidden');
      document.getElementById('select-screen').classList.remove('hidden');
      this.gameState = 'SELECT';
    });

    const cards = document.querySelectorAll('.char-card');
    cards.forEach(card => {
      card.addEventListener('click', () => {
        cards.forEach(c => c.classList.remove('selected'));
        card.classList.add('selected');
        this.selectedClass = card.getAttribute('data-class');
        this.audio.play('button.wav');

        if (this.selectedClass === 'fighter') this.classFile = 'levelsb.dat';
        else if (this.selectedClass === 'cleric') this.classFile = 'levelsc.dat';
        else if (this.selectedClass === 'thief') this.classFile = 'levelst.dat';
        else if (this.selectedClass === 'wizard') this.classFile = 'levelsw.dat';
      });
    });

    document.getElementById('btn-select-confirm').addEventListener('click', () => {
      this.audio.play('spawn.wav');
      document.getElementById('select-screen').classList.add('hidden');
      document.getElementById('hud-panel').classList.remove('hidden');
      this.startNewGame();
    });

    document.getElementById('btn-ring').addEventListener('click', () => this.useRing());
    document.getElementById('btn-drop').addEventListener('click', () => this.dropTreasure());
    document.getElementById('btn-help').addEventListener('click', () => this.showHelp());
    document.getElementById('btn-obj').addEventListener('click', () => this.showObjective());
    document.getElementById('modal-close').addEventListener('click', () => this.closeModal());

    window.addEventListener('mousedown', (e) => {
      if (this.gameState !== 'PLAYING') return;
      if (e.target.closest('#hud-panel') || e.target.closest('#modal-screen')) return;

      if (e.button === 0) {
        const grid = this.renderer.screenToGrid(e.clientX, e.clientY);
        if (grid) {
          this.movePlayerTowards(grid.x, grid.y);
        }
      } else if (e.button === 2) {
        e.preventDefault();
        this.playerFire();
      }
    });

    window.addEventListener('contextmenu', (e) => e.preventDefault());

    window.addEventListener('dblclick', (e) => {
      if (this.gameState !== 'PLAYING') return;
      this.checkStairs();
    });

    window.addEventListener('keydown', (e) => {
      this.keys[e.key] = true;

      if (this.gameState === 'PLAYING') {
        if (e.key === 'Control' || e.code === 'Space') {
          e.preventDefault();
          this.playerFire();
        } else if (e.key === 'Shift') {
          e.preventDefault();
          this.checkStairs();
        } else if (e.key === 'r' || e.key === 'R') {
          this.useRing();
        } else if (e.key === 't' || e.key === 'T') {
          this.dropTreasure();
        } else if (e.key === 'F1') {
          e.preventDefault();
          this.showHelp();
        } else if (e.key === 'F2') {
          e.preventDefault();
          this.showObjective();
        } else if (e.key === 'ArrowUp' || e.key === 'w' || e.key === 'W') {
          this.playerStep(DIR.NE);
        } else if (e.key === 'ArrowDown' || e.key === 's' || e.key === 'S') {
          this.playerStep(DIR.SW);
        } else if (e.key === 'ArrowLeft' || e.key === 'a' || e.key === 'A') {
          this.playerStep(DIR.NW);
        } else if (e.key === 'ArrowRight' || e.key === 'd' || e.key === 'D') {
          this.playerStep(DIR.SE);
        }
      } else if (this.gameState === 'MODAL' && (e.key === 'Escape' || e.key === 'Enter')) {
        this.closeModal();
      }
    });

    window.addEventListener('keyup', (e) => {
      this.keys[e.key] = false;
    });
  }

  startNewGame() {
    this.currentLevel = 1;
    this.currentEpisode = 1;
    this.loadDungeonFloor(1, true);
    this.gameState = 'PLAYING';
    this.showMessage(`Welcome to Murk! Floor ${this.currentLevel}`);
    this.audio.play('alive.wav');
  }

  loadDungeonFloor(floorNum, isFirstEpisode = false) {
    this.currentLevel = floorNum;
    const ep = this.dungeonSys.buildNewLevel(this.classFile, this.currentEpisode);

    if (isFirstEpisode || !this.player) {
      const spawn = this.dungeonSys.findSpawnLocation(floorNum);
      let imgId = 1;
      let hp = 45;
      let speed = 4.2;
      let dmin = 2, dmax = 4;

      if (this.selectedClass === 'fighter') { imgId = 1; hp = 45; speed = 4.0; dmin = 2; dmax = 4; }
      else if (this.selectedClass === 'cleric') { imgId = 5; hp = 40; speed = 4.2; dmin = 1; dmax = 3; }
      else if (this.selectedClass === 'thief') { imgId = 3; hp = 35; speed = 5.0; dmin = 1; dmax = 3; }
      else if (this.selectedClass === 'wizard') { imgId = 4; hp = 30; speed = 4.4; dmin = 2; dmax = 5; }

      this.player = new Entity({
        name: 'Player',
        type: 'Player',
        image: imgId,
        x: spawn.x,
        y: spawn.y,
        lvl: floorNum,
        ability: hp,
        moveSpeed: speed,
        damagemin: dmin,
        damagemax: dmax,
        ring: 1,
        armour: 10,
        team: 1
      });
    } else {
      const spawn = this.dungeonSys.findSpawnLocation(floorNum);
      this.player.mapx = spawn.x;
      this.player.mapy = spawn.y;
      this.player.prevX = spawn.x;
      this.player.prevY = spawn.y;
      this.player.level = floorNum;
      this.player.moveProgress = 1.0;
    }

    this.monsters = [];
    this.projectiles = [];

    const spawnList = [
      { type: 'Beholder', image: 2, count: ep.beholders, hp: 30, spd: 3.0, dmin: 2, dmax: 4 },
      { type: 'Mouther', image: 7, count: ep.mouther, hp: 12, spd: 3.2, dmin: 1, dmax: 2 },
      { type: 'Orc', image: 8, count: ep.ogre, hp: 20, spd: 3.5, dmin: 1, dmax: 3 },
      { type: 'Skeleton', image: 6, count: ep.skeletons, hp: 14, spd: 3.0, dmin: 1, dmax: 2 },
      { type: 'DeathOrb', image: 9, count: ep.deathorb, hp: 50, spd: 2.5, dmin: 3, dmax: 6 }
    ];

    spawnList.forEach(m => {
      for (let k = 0; k < m.count; k++) {
        const loc = this.dungeonSys.findSpawnLocation(floorNum);
        if (Math.abs(loc.x - this.player.mapx) > 1 || Math.abs(loc.y - this.player.mapy) > 1) {
          this.monsters.push(new Entity({
            name: m.type,
            type: 'Monster',
            image: m.image,
            x: loc.x,
            y: loc.y,
            lvl: floorNum,
            ability: m.hp,
            moveSpeed: m.spd,
            damagemin: m.dmin,
            damagemax: m.dmax,
            team: 0
          }));
        }
      }
    });

    this.renderer.buildDungeonMesh(this.dungeonSys, floorNum);
    this.updateHUD();
  }

  playerStep(dir) {
    if (!this.player || !this.player.active || this.player.moveProgress < 1.0) return;

    this.player.direction = dir;
    let nx = this.player.mapx;
    let ny = this.player.mapy;

    if (dir === DIR.NW) nx--;
    else if (dir === DIR.NE) ny--;
    else if (dir === DIR.SW) ny++;
    else if (dir === DIR.SE) nx++;

    const cell = this.dungeonSys.dungeon[nx] &&
                 this.dungeonSys.dungeon[nx][ny] &&
                 this.dungeonSys.dungeon[nx][ny][this.currentLevel];

    if (cell && (cell.type === 'f' || cell.type === 's')) {
      const blocked = this.monsters.some(m => m.active && m.mapx === nx && m.mapy === ny);
      if (!blocked) {
        this.player.prevX = this.player.mapx;
        this.player.prevY = this.player.mapy;
        this.player.mapx = nx;
        this.player.mapy = ny;
        this.player.moveProgress = 0.0;
        this.audio.play('walk.wav', 0.3);

        this.checkCellInteractions(nx, ny);
      }
    }
  }

  movePlayerTowards(targetX, targetY) {
    if (!this.player || this.player.moveProgress < 1.0) return;

    const dx = targetX - this.player.mapx;
    const dy = targetY - this.player.mapy;

    if (dx === 0 && dy === 0) return;

    let dir = this.player.direction;
    if (Math.abs(dx) >= Math.abs(dy)) {
      dir = dx < 0 ? DIR.NW : DIR.SE;
    } else {
      dir = dy < 0 ? DIR.NE : DIR.SW;
    }
    this.playerStep(dir);
  }

  playerFire() {
    if (!this.player || !this.player.active || this.player.fireTimer > 0) return;

    this.player.fireTimer = 0.35;
    const proj = new Projectile(
      this.player.mapx,
      this.player.mapy,
      this.currentLevel,
      this.player.direction,
      this.player
    );
    this.projectiles.push(proj);

    if (this.player.image === 4 || this.player.image === 5) {
      this.audio.play('fireball.wav');
    } else if (this.player.image === 3) {
      this.audio.play('tsword.wav');
    } else {
      this.audio.play('throw.wav');
    }
  }

  useRing() {
    if (!this.player || this.player.ring <= 0 || this.player.protection > 0) return;

    this.player.ring--;
    this.player.protection = 8.0;
    this.audio.play('protection.wav');
    this.audio.play('tcleric.wav');
    this.showMessage('Shield of Protection activated!');
    this.updateHUD();
  }

  dropTreasure() {
    if (!this.player) return;

    if (this.player.hasTreasure1) {
      this.player.hasTreasure1 = false;
      this.dungeonSys.foundtreasure = 0;
      this.dungeonSys.treasurex = this.player.mapx;
      this.dungeonSys.treasurey = this.player.mapy;
      this.dungeonSys.treasurelvl = this.currentLevel;
      this.showMessage('Relic dropped on floor.');
      this.audio.play('button.wav');
      this.renderer.buildDungeonMesh(this.dungeonSys, this.currentLevel);
      this.updateHUD();
    } else if (this.player.hasTreasure2) {
      this.player.hasTreasure2 = false;
      this.dungeonSys.foundtreasure2 = 0;
      this.dungeonSys.treasurex2 = this.player.mapx;
      this.dungeonSys.treasurey2 = this.player.mapy;
      this.dungeonSys.treasurelvl2 = this.currentLevel;
      this.showMessage('Deep Relic dropped on floor.');
      this.audio.play('button.wav');
      this.renderer.buildDungeonMesh(this.dungeonSys, this.currentLevel);
      this.updateHUD();
    }
  }

  checkStairs() {
    if (!this.player) return;
    const cell = this.dungeonSys.dungeon[this.player.mapx][this.player.mapy][this.currentLevel];

    if (cell && cell.type === 's') {
      const isUp = cell.item === 1;
      if (isUp) {
        if (this.currentLevel > 1) {
          this.currentLevel--;
          this.audio.play('win.wav');
          this.showMessage(`Ascended to Floor ${this.currentLevel}!`);
          this.loadDungeonFloor(this.currentLevel);
        }
      } else {
        if (this.currentLevel < this.dungeonSys.maxlevel) {
          this.currentLevel++;
          this.audio.play('win.wav');
          this.showMessage(`Descended to Floor ${this.currentLevel}!`);
          this.loadDungeonFloor(this.currentLevel);
        } else {
          if (this.player.hasTreasure1 || this.player.hasTreasure2) {
            this.levelCompleted();
          } else {
            this.showMessage('Retrieve the ancient Relic before completing episode!');
          }
        }
      }
    }
  }

  checkCellInteractions(x, y) {
    const cell = this.dungeonSys.dungeon[x][y][this.currentLevel];
    if (!cell) return;

    if (cell.item && cell.item !== 0 && cell.item !== 1 && cell.item !== 'c') {
      if (cell.item === 'e') {
        this.player.ability = Math.min(this.player.maxAbility + 20, this.player.ability + 15);
        this.showMessage('Drank Elixir! Health restored.');
        this.audio.play('item.wav');
      } else if (cell.item === 'a') {
        this.player.armour += 10;
        this.showMessage('Equipped Enchanted Armour (+10 Armour).');
        this.audio.play('item.wav');
      } else if (cell.item === 'r') {
        this.player.ring++;
        this.showMessage('Found Ring of Protection (+1 Ring).');
        this.audio.play('item.wav');
      } else if (cell.item === 's') {
        this.player.skill += 10;
        this.showMessage('Knowledge tome (+10 Skill).');
        this.audio.play('win.wav');
      }
      cell.item = 0;
      this.renderer.buildDungeonMesh(this.dungeonSys, this.currentLevel);
      this.updateHUD();
    }

    if (x === this.dungeonSys.treasurex && y === this.dungeonSys.treasurey && this.currentLevel === this.dungeonSys.treasurelvl && !this.player.hasTreasure1) {
      this.player.hasTreasure1 = true;
      this.dungeonSys.foundtreasure = 1;
      this.showMessage('Discovered the Ancient Relic! Return it home.');
      this.audio.play('treasure.wav');
      this.renderer.buildDungeonMesh(this.dungeonSys, this.currentLevel);
      this.updateHUD();
    }

    if (x === this.dungeonSys.treasurex2 && y === this.dungeonSys.treasurey2 && this.currentLevel === this.dungeonSys.treasurelvl2 && !this.player.hasTreasure2) {
      this.player.hasTreasure2 = true;
      this.dungeonSys.foundtreasure2 = 1;
      this.showMessage('Found the Deep Relic of Murk!');
      this.audio.play('treasure.wav');
      this.renderer.buildDungeonMesh(this.dungeonSys, this.currentLevel);
      this.updateHUD();
    }
  }

  levelCompleted() {
    this.audio.play('win.wav');
    this.player.skill += 50;
    this.player.armour += 20;
    this.player.ring += 2;
    this.currentEpisode++;
    this.showMessage(`Episode ${this.currentEpisode - 1} Complete!`);

    this.showModal('EPISODE COMPLETE!',
      `Magnificent victory!\nYou recovered the relic and conquered the depths.\n\nAdvancing to Episode ${this.currentEpisode}...`
    );

    this.player.hasTreasure1 = false;
    this.player.hasTreasure2 = false;
    this.loadDungeonFloor(1);
  }

  updateMonsterAI(dt) {
    this.monsterAITimer += dt;
    if (this.monsterAITimer < 0.35) return;
    this.monsterAITimer = 0;

    if (!this.player || !this.player.active) return;

    this.monsters.forEach(m => {
      if (!m.active || m.dead > 0 || m.moveProgress < 1.0) return;

      const dist = Math.abs(m.mapx - this.player.mapx) + Math.abs(m.mapy - this.player.mapy);

      if (dist <= 6 && Math.random() < 0.25 && m.fireTimer <= 0) {
        m.fireTimer = 1.5;
        let fireDir = DIR.SE;
        const dx = this.player.mapx - m.mapx;
        const dy = this.player.mapy - m.mapy;
        if (Math.abs(dx) >= Math.abs(dy)) {
          fireDir = dx < 0 ? DIR.NW : DIR.SE;
        } else {
          fireDir = dy < 0 ? DIR.NE : DIR.SW;
        }
        m.direction = fireDir;

        const proj = new Projectile(m.mapx, m.mapy, this.currentLevel, fireDir, m);
        this.projectiles.push(proj);

        if (m.image === 2) this.audio.play('orb.wav');
        else if (m.image === 6) this.audio.play('skeleton.wav');
        else if (m.image === 7) this.audio.play('mouther.wav');
        else if (m.image === 9) this.audio.play('deathorb.wav');
        return;
      }

      let stepX = 0;
      let stepY = 0;

      if (dist < 8) {
        if (this.player.mapx < m.mapx) stepX = -1;
        else if (this.player.mapx > m.mapx) stepX = 1;
        if (this.player.mapy < m.mapy) stepY = -1;
        else if (this.player.mapy > m.mapy) stepY = 1;
      } else {
        const r = Math.random();
        if (r < 0.25) stepX = -1;
        else if (r < 0.5) stepX = 1;
        else if (r < 0.75) stepY = -1;
        else stepY = 1;
      }

      let targetDir = m.direction;
      let nx = m.mapx;
      let ny = m.mapy;

      if (Math.random() < 0.5 && stepX !== 0) {
        nx += stepX;
        targetDir = stepX < 0 ? DIR.NW : DIR.SE;
      } else if (stepY !== 0) {
        ny += stepY;
        targetDir = stepY < 0 ? DIR.NE : DIR.SW;
      }

      const cell = this.dungeonSys.dungeon[nx] &&
                   this.dungeonSys.dungeon[nx][ny] &&
                   this.dungeonSys.dungeon[nx][ny][this.currentLevel];

      if (cell && (cell.type === 'f' || cell.type === 's')) {
        if (nx === this.player.mapx && ny === this.player.mapy) {
          const dmg = Math.floor(Math.random() * (m.damagemax - m.damagemin + 1)) + m.damagemin;
          this.player.takeDamage(dmg);
          this.audio.play('ouch.wav');
          this.updateHUD();
          if (this.player.ability <= 0) {
            this.handlePlayerDeath();
          }
        } else {
          const blocked = this.monsters.some(other => other !== m && other.active && other.mapx === nx && other.mapy === ny);
          if (!blocked) {
            m.direction = targetDir;
            m.prevX = m.mapx;
            m.prevY = m.mapy;
            m.mapx = nx;
            m.mapy = ny;
            m.moveProgress = 0.0;
          }
        }
      }
    });
  }

  handlePlayerDeath() {
    this.audio.play('dead.wav');
    this.showMessage('YOU HAVE DIED IN THE DUNGEON!');
    this.showModal('VALIANT EFFORT',
      `You succumbed to the dark horrors of Murk.\n\nScore / Skill: ${this.player.skill}\nFloors Explored: ${this.currentLevel}\n\nClick Continue to resurrect.`
    );
    this.gameState = 'MODAL';
  }

  updateHUD() {
    if (!this.player) return;
    document.getElementById('hud-floor').textContent = this.currentLevel;
    document.getElementById('hud-hp').textContent = Math.round(this.player.ability);
    document.getElementById('hud-armour').textContent = this.player.armour;
    document.getElementById('hud-skill').textContent = this.player.skill;
    document.getElementById('hud-rings').textContent = this.player.ring;

    const ringBtn = document.getElementById('btn-ring');
    if (this.player.protection > 0) {
      ringBtn.classList.add('active');
    } else {
      ringBtn.classList.remove('active');
    }
  }

  showMessage(msg) {
    const banner = document.getElementById('message-banner');
    banner.textContent = msg;
    banner.classList.remove('hidden');
    clearTimeout(this.msgTimeout);
    this.msgTimeout = setTimeout(() => {
      banner.classList.add('hidden');
    }, 3000);
  }

  showHelp() {
    this.showModal('MURK INSTRUCTIONS',
      `MOUSE CONTROLS:
• Left Click: Move towards dungeon cell
• Right Click: Shoot / Attack
• Double Click: Ascend / Descend Stairs or Ladders

KEYBOARD CONTROLS:
• Arrow Keys / WASD: Move
• Ctrl / Space: Fire Missile
• Shift: Climb Stairs
• [R]: Activate Ring of Protection (Invulnerability)
• [T]: Drop Carried Relic
• [F1]: Help Screen | [F2]: Episode Goal

OBJECTIVE:
Traverse the perilous dungeon floors, slay foul beasts, collect treasure relics, and survive the Murk!`
    );
  }

  showObjective() {
    const ep = this.dungeonSys.currentEpisodeDef;
    const title = ep ? ep.title : 'Episode Objective';
    const text = ep ? ep.response : 'Find the ancient Relic and return it safely!';
    this.showModal(title.toUpperCase(), text);
  }

  showModal(title, text) {
    document.getElementById('modal-title').textContent = title;
    document.getElementById('modal-content').textContent = text;
    document.getElementById('modal-screen').classList.remove('hidden');
    this.gameState = 'MODAL';
  }

  closeModal() {
    document.getElementById('modal-screen').classList.add('hidden');
    if (this.player && this.player.ability <= 0) {
      this.startNewGame();
    } else {
      this.gameState = 'PLAYING';
    }
  }

  gameLoop(time) {
    requestAnimationFrame((t) => this.gameLoop(t));

    const dt = Math.min(0.1, (time - this.lastTime) / 1000) || 0.016;
    this.lastTime = time;

    if (this.gameState === 'PLAYING') {
      if (this.player) {
        this.player.update(dt, this.dungeonSys);
        this.renderer.followPlayer(this.player);
      }

      this.updateMonsterAI(dt);
      this.monsters.forEach(m => m.update(dt, this.dungeonSys));

      const entities = [this.player, ...this.monsters];
      for (let i = this.projectiles.length - 1; i >= 0; i--) {
        const p = this.projectiles[i];
        p.update(dt, this.dungeonSys, entities, (proj, target, dmg) => {
          this.audio.play('impact.wav');
          if (target === this.player) {
            this.audio.play('ouch.wav');
            this.updateHUD();
            if (this.player.ability <= 0) this.handlePlayerDeath();
          } else {
            if (target.ability <= 0) {
              this.audio.play('die.wav');
              this.player.skill += 15;
              this.updateHUD();
            }
          }
        });
        if (!p.active) {
          this.projectiles.splice(i, 1);
        }
      }

      this.renderer.updateEntities(entities, this.player);
      this.renderer.updateProjectiles(this.projectiles);
    }

    this.renderer.render();
  }
}

window.addEventListener('DOMContentLoaded', () => {
  window.murkGame = new MurkGame();
});
