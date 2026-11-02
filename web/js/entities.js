/**
 * Murk Entities: Player, Monsters, Projectiles
 * Faithful implementation of character images, stats, speeds, AI behaviors, and projectile physics.
 */

const DIR = {
  NW: 1, // left/up
  NE: 2, // right/up
  SW: 3, // left/down
  SE: 4  // right/down
};

class Entity {
  constructor(config = {}) {
    this.name = config.name || 'Entity';
    this.type = config.type || 'Monster';
    this.image = config.image || 1;

    this.mapx = config.x || 1;
    this.mapy = config.y || 1;
    this.prevX = this.mapx;
    this.prevY = this.mapy;
    this.moveProgress = 1.0;
    this.moveSpeed = config.moveSpeed || 4.0;

    this.level = config.lvl || 1;
    this.direction = config.dir || DIR.SE;
    this.frame = 0;
    this.animTimer = 0;

    this.ability = config.ability || 30;
    this.maxAbility = this.ability;
    this.damagemin = config.damagemin || 1;
    this.damagemax = config.damagemax || 3;
    this.maxmissle = config.maxmissle || 4;
    this.firerate = config.firerate || 5;
    this.fireTimer = 0;

    this.armour = config.armour || 0;
    this.ring = config.ring || 0;
    this.protection = 0;
    this.skill = 0;
    this.dead = 0;
    this.hitTimer = 0;
    this.active = true;

    this.copx = this.mapx;
    this.copy = this.mapy;
    this.coplvl = this.level;
    this.asleep = config.asleep || false;
    this.intelligence = config.intelligence || 1;
    this.team = config.team || 0;

    this.hasTreasure1 = false;
    this.hasTreasure2 = false;
  }

  getWorldPosition(renderer) {
    const p1 = renderer.gridToWorld(this.prevX, this.prevY);
    const p2 = renderer.gridToWorld(this.mapx, this.mapy);
    const t = Math.min(1.0, this.moveProgress);
    return {
      x: p1.x + (p2.x - p1.x) * t,
      z: p1.z + (p2.z - p1.z) * t
    };
  }

  getSpriteUV() {
    let framex = 0;
    let framey = 0;
    let frame2x = 63;
    let frame2y = 63;

    let adjust = 0;
    const animFrame = this.frame % 6;
    if (animFrame === 1) adjust = 64;
    else if (animFrame === 2) adjust = 128;
    else if (animFrame === 3) adjust = 192;
    else if (animFrame === 4) adjust = 256;
    else if (animFrame === 5) adjust = 320;

    if (this.image === 9) adjust = 0;
    if (this.image === 2) adjust = 64;

    switch (this.direction) {
      case DIR.NW:
        framex = 0 + adjust;
        framey = 0;
        frame2x = 63 + adjust;
        frame2y = 63;
        break;
      case DIR.NE:
        framex = 384 + adjust;
        framey = 0;
        frame2x = 447 + adjust;
        frame2y = 63;
        break;
      case DIR.SW:
        framex = 0 + adjust;
        framey = 64;
        frame2x = 63 + adjust;
        frame2y = 127;
        break;
      case DIR.SE:
        framex = 384 + adjust;
        framey = 64;
        frame2x = 447 + adjust;
        frame2y = 127;
        break;
    }

    switch (this.image) {
      case 1: framey += 0; frame2y += 0; break;
      case 2: framey += 128; frame2y += 128; break;
      case 3: framey += 256; frame2y += 256; break;
      case 4: framey += 384; frame2y += 384; break;
      case 5: framey += 512; frame2y += 512; break;
      case 6: framey += 768; frame2y += 768; break;
      case 7: framey += 896; frame2y += 896; break;
      case 8: framey += 1024; frame2y += 1024; break;
      case 9: framey += 128; frame2y += 128; break;
    }

    return [framex, framey, frame2x, frame2y];
  }

  update(dt, dungeonSys) {
    if (this.moveProgress < 1.0) {
      this.moveProgress += dt * this.moveSpeed;
      if (this.moveProgress >= 1.0) {
        this.moveProgress = 1.0;
        this.prevX = this.mapx;
        this.prevY = this.mapy;
        this.frame = 0;
      } else {
        this.animTimer += dt;
        if (this.animTimer > 0.12) {
          this.animTimer = 0;
          this.frame = (this.frame + 1) % 5 + 1;
        }
      }
    }

    if (this.fireTimer > 0) this.fireTimer -= dt;
    if (this.hitTimer > 0) this.hitTimer -= dt;
    if (this.protection > 0) this.protection -= dt;
  }

  takeDamage(amount) {
    if (this.protection > 0) return 0;

    let effectiveDamage = amount;
    if (this.armour > 0) {
      const absorbed = Math.min(this.armour, Math.floor(amount / 2));
      this.armour -= absorbed;
      effectiveDamage = Math.max(1, amount - absorbed);
    }

    this.ability -= effectiveDamage;
    this.hitTimer = 0.25;

    if (this.ability <= 0) {
      this.ability = 0;
      this.dead = 64;
      this.active = false;
    }
    return effectiveDamage;
  }
}

class Projectile {
  constructor(x, y, lvl, dir, owner) {
    this.mapx = x;
    this.mapy = y;
    this.level = lvl;
    this.direction = dir;
    this.owner = owner;
    this.speed = 10.0;
    this.progress = 0;
    this.image = 1;

    if (owner.image === 1) this.image = 1;
    else if (owner.image === 2) this.image = 3;
    else if (owner.image === 3) this.image = 1;
    else if (owner.image === 4) this.image = 3;
    else if (owner.image === 5) this.image = 4;
    else if (owner.image === 6) this.image = 1;
    else if (owner.image === 7) this.image = 6;
    else if (owner.image === 8) this.image = 2;
    else if (owner.image === 9) this.image = 4;

    this.active = true;
    this.distanceTraveled = 0;
    this.maxDistance = 12;
  }

  getWorldPosition(renderer) {
    const worldPos = renderer.gridToWorld(this.mapx, this.mapy);
    let offX = 0;
    let offZ = 0;
    const factor = renderer.tileSize * 0.7071 * this.progress;

    if (this.direction === DIR.NW) { offX = -factor; offZ = -factor; }
    else if (this.direction === DIR.NE) { offX = factor; offZ = -factor; }
    else if (this.direction === DIR.SW) { offX = -factor; offZ = factor; }
    else if (this.direction === DIR.SE) { offX = factor; offZ = factor; }

    return { x: worldPos.x + offX, z: worldPos.z + offZ };
  }

  getSpriteUV() {
    let adjust = (this.image - 1) * 64;
    let framex = 0;
    let framey = 640;
    let frame2x = 63;
    let frame2y = 703;

    switch (this.direction) {
      case DIR.NW: framex = 0 + adjust; break;
      case DIR.NE: framex = 384 + adjust; break;
      case DIR.SW: framex = 0 + adjust; framey += 64; frame2y += 64; break;
      case DIR.SE: framex = 384 + adjust; framey += 64; frame2y += 64; break;
    }
    frame2x = framex + 63;
    return [framex, framey, frame2x, frame2y];
  }

  update(dt, dungeonSys, entities, onHit) {
    this.progress += dt * this.speed;

    while (this.progress >= 1.0) {
      this.progress -= 1.0;
      this.distanceTraveled++;

      if (this.direction === DIR.NW) this.mapx--;
      else if (this.direction === DIR.NE) this.mapy--;
      else if (this.direction === DIR.SW) this.mapy++;
      else if (this.direction === DIR.SE) this.mapx++;

      const cell = dungeonSys.dungeon[this.mapx] &&
                   dungeonSys.dungeon[this.mapx][this.mapy] &&
                   dungeonSys.dungeon[this.mapx][this.mapy][this.level];

      if (!cell || (cell.type !== 'f' && cell.type !== 's')) {
        this.active = false;
        return;
      }

      for (const ent of entities) {
        if (!ent.active || ent.level !== this.level) continue;
        if (ent === this.owner) continue;
        if (ent.team === this.owner.team) continue;

        if (ent.mapx === this.mapx && ent.mapy === this.mapy) {
          const dmg = Math.floor(Math.random() * (this.owner.damagemax - this.owner.damagemin + 1)) + this.owner.damagemin;
          ent.takeDamage(dmg);
          if (onHit) onHit(this, ent, dmg);
          this.active = false;
          return;
        }
      }

      if (this.distanceTraveled >= this.maxDistance) {
        this.active = false;
        return;
      }
    }
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { DIR, Entity, Projectile };
}
