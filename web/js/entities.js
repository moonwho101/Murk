/**
 * Murk entities: characters, monsters and missiles, with the stat tables from the original game.
 */

// Row of each creature in p256.png (two 64px rows per creature: back-facing then front-facing).
const SPRITE_ROW = { 1: 0, 2: 128, 3: 256, 4: 384, 5: 512, 6: 768, 7: 896, 8: 1024, 9: 128 };

// Base time (seconds) to cross one tile; matches the original 6 frames at 50ms.
const BASE_TILE_TIME = 0.28;

const CLASS_DEFS = {
  fighter: {
    name: 'Warrior', image: 1, file: 'levelsb.dat', pad: 0,
    hp: 40, dmin: 2, dmax: 3, shots: 4, fireCd: 0.30, tileTime: 0.29, rings: 1, armour: 5,
    wearsArmour: true, readsScrolls: false, fireSound: 'throw.wav'
  },
  cleric: {
    name: 'Cleric', image: 5, file: 'levelsc.dat', pad: 3,
    hp: 36, dmin: 1, dmax: 3, shots: 4, fireCd: 0.30, tileTime: 0.28, rings: 2, armour: 0,
    wearsArmour: false, readsScrolls: true, fireSound: 'fireball.wav'
  },
  thief: {
    name: 'Thief', image: 3, file: 'levelst.dat', pad: 1,
    hp: 34, dmin: 1, dmax: 3, shots: 4, fireCd: 0.22, tileTime: 0.24, rings: 1, armour: 3,
    wearsArmour: true, readsScrolls: false, fireSound: 'tsword.wav'
  },
  wizard: {
    name: 'Wizard', image: 4, file: 'levelsw.dat', pad: 2,
    hp: 30, dmin: 2, dmax: 4, shots: 4, fireCd: 0.34, tileTime: 0.28, rings: 1, armour: 0,
    wearsArmour: false, readsScrolls: true, fireSound: 'fireball.wav'
  }
};

const CLASS_BY_IMAGE = { 1: 'fighter', 5: 'cleric', 3: 'thief', 4: 'wizard' };

// Monster stats from MurkInit.cpp: hp, damage range, missile cap, fire delay, how eagerly it opens fire.
const MONSTER_DEFS = {
  beholder: { name: 'Beholder', image: 2, hp: 30, dmin: 2, dmax: 4, shots: 3, fireCd: 0.55, aggr: 0.7, sound: 'orb.wav', wake: 'dragon.wav' },
  mouther: { name: 'Gibbering Mouther', image: 7, hp: 6, dmin: 1, dmax: 1, shots: 2, fireCd: 0.8, aggr: 0.9, sound: 'mouther.wav', wake: 'mouther.wav' },
  ogre: { name: 'Brute', image: 8, hp: 15, dmin: 1, dmax: 3, shots: 4, fireCd: 0.7, aggr: 0.7, sound: 'throw2.wav', wake: 'dragon.wav' },
  skeleton: { name: 'Skeleton', image: 6, hp: 10, dmin: 1, dmax: 1, shots: 2, fireCd: 0.9, aggr: 0.8, sound: 'skeleton.wav', wake: 'skeleton.wav' },
  deathorb: { name: 'Death Orb', image: 9, hp: 50, dmin: 2, dmax: 6, shots: 8, fireCd: 0.3, aggr: 0.7, sound: 'deathorb.wav', wake: 'deathorb.wav' }
};

let entityIdCounter = 1;

class Entity {
  constructor(cfg) {
    this.id = entityIdCounter++;
    this.kind = cfg.kind || 'monster'; // player | ally | rival | monster
    this.type = cfg.type || 'monster';
    this.name = cfg.name || 'Creature';
    this.image = cfg.image;
    this.team = cfg.team || 0;

    this.x = cfg.x;
    this.y = cfg.y;
    this.lvl = cfg.lvl;
    this.fromX = this.x;
    this.fromY = this.y;
    this.dir = cfg.dir || DIR.SE;
    this.moveT = 1;
    this.tileTime = cfg.tileTime || BASE_TILE_TIME;

    this.maxHp = cfg.hp;
    this.hp = cfg.hp;
    this.armour = cfg.armour || 0;
    this.rings = cfg.rings || 0;
    this.protection = 0;
    this.skill = 0;
    this.dmin = cfg.dmin;
    this.dmax = cfg.dmax;
    this.maxShots = cfg.shots;
    this.fireCd = cfg.fireCd;
    this.fireTimer = 0;
    this.shots = 0;
    this.fireSound = cfg.fireSound || null;
    this.aggr = cfg.aggr || 0.8;

    this.asleep = !!cfg.asleep;
    this.alive = true;
    this.deathTimer = 0;
    this.respawnTimer = 0;
    this.hitFlash = 0;
    this.restTimer = 0;
    this.thinkTimer = Math.random() * 0.3;
    this.shotFlash = 0;
    this.spawn = { x: this.x, y: this.y, lvl: this.lvl };
    this.lair = cfg.lair || { x: this.x, y: this.y, lvl: this.lvl };
    this.classKey = cfg.classKey || null;
    this.carrying = 0;
    this.bob = Math.random() * 6.28;
  }

  get isMonster() {
    return this.kind === 'monster';
  }

  get moving() {
    return this.moveT < 1;
  }

  // Renders from the tile it left toward the tile it is entering.
  renderCell() {
    const t = this.moveT;
    return {
      x: this.fromX + (this.x - this.fromX) * t,
      y: this.fromY + (this.y - this.fromY) * t
    };
  }

  startMove(nx, ny, dir) {
    this.fromX = this.x;
    this.fromY = this.y;
    this.x = nx;
    this.y = ny;
    this.dir = dir;
    this.moveT = 0;
  }

  teleport(x, y, lvl) {
    this.x = x;
    this.y = y;
    this.fromX = x;
    this.fromY = y;
    this.lvl = lvl;
    this.moveT = 1;
  }

  // Returns true on the frame the entity finishes entering its tile.
  update(dt) {
    let arrived = false;
    if (this.moveT < 1) {
      this.moveT = Math.min(1, this.moveT + dt / this.tileTime);
      if (this.moveT >= 1) {
        this.fromX = this.x;
        this.fromY = this.y;
        arrived = true;
      }
    }
    if (this.fireTimer > 0) this.fireTimer -= dt;
    if (this.hitFlash > 0) this.hitFlash -= dt;
    if (this.shotFlash > 0) this.shotFlash -= dt;
    if (this.restTimer > 0) this.restTimer -= dt;
    if (this.protection > 0) this.protection = Math.max(0, this.protection - dt);
    return arrived;
  }

  // Source rectangle [x, y, w, h] in the creature sheet.
  spriteRect() {
    let col = 0;
    if (this.image === 9) col = 1;
    else if (this.image !== 2 && this.moveT < 1) col = 1 + Math.min(4, Math.floor(this.moveT * 5));

    const east = this.dir === DIR.NE || this.dir === DIR.SE;
    const front = this.dir === DIR.SW || this.dir === DIR.SE;
    return [
      (east ? 384 : 0) + col * 64,
      SPRITE_ROW[this.image] + (front ? 64 : 0),
      64, 64
    ];
  }

  rollDamage() {
    return this.dmin + Math.floor(Math.random() * (this.dmax - this.dmin + 1));
  }

  // Armour soaks damage first (as in the original). Returns { hp, armour, blocked }.
  takeHit(amount) {
    if (this.protection > 0) return { hp: 0, armour: 0, blocked: true };
    let left = amount;
    let armourLost = 0;
    if (this.armour > 0) {
      armourLost = Math.min(this.armour, left);
      this.armour -= armourLost;
      left -= armourLost;
    }
    this.hp -= left;
    this.hitFlash = 0.18;
    return { hp: left, armour: armourLost, blocked: false };
  }
}

// Missile sprites in the sheet: 1 dagger, 2 axe, 3 fireball, 4 frost bolt, 5 rock, 6 venom.
function missileKindFor(image, ownerKind) {
  switch (image) {
    case 1: return 1;
    case 2: return 3;
    case 3: return 1;
    case 4: return 3;
    case 5: return 4;
    case 6: return 1;
    case 7: return 6;
    case 8: return Math.random() < 0.5 ? 2 : 5;
    case 9: return 4;
    default: return 1;
  }
}

class Projectile {
  constructor(owner, dir) {
    this.owner = owner;
    this.dir = dir;
    this.x = owner.x;
    this.y = owner.y;
    this.lvl = owner.lvl;
    this.t = 0;
    this.speed = 7;
    this.travelled = 0;
    this.maxRange = 11;
    this.kind = missileKindFor(owner.image);
    this.alive = true;
    this.spin = Math.random() * 6.28;
  }

  renderCell() {
    const v = DIR_VEC[this.dir];
    return { x: this.x + v[0] * this.t, y: this.y + v[1] * this.t };
  }

  spriteRect() {
    const east = this.dir === DIR.NE || this.dir === DIR.SE;
    const front = this.dir === DIR.SW || this.dir === DIR.SE;
    return [(east ? 384 : 0) + (this.kind - 1) * 64, 640 + (front ? 64 : 0), 64, 64];
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { Entity, Projectile, CLASS_DEFS, CLASS_BY_IMAGE, MONSTER_DEFS, BASE_TILE_TIME };
}
