/**
 * Murk Dungeon Generator and Level System
 * Based on original C++ logic from MurkInit.cpp and MurkThink.cpp
 */

class DungeonSystem {
  constructor() {
    this.DUNGEONX = 14;
    this.DUNGEONY = 14;
    this.maxlevel = 3;
    this.dungeon = {}; // [x][y][lvl] -> { type: 'f'|'s'|'u'|0, item: 0|'c'|'e'|'s'|'r'|'a'|1, explored: 0|1, image: 0..4 }
    this.pits = [];
    this.dimage = [];
    this.tile_type = [];
    this.treasurex = 0;
    this.treasurey = 0;
    this.treasurelvl = 1;
    this.foundtreasure = 0;

    this.treasurex2 = 0;
    this.treasurey2 = 0;
    this.treasurelvl2 = 1;
    this.foundtreasure2 = 0;

    this.levelDefinitions = {};
    this.currentEpisodeDef = null;
  }

  loadLevelDefs(levelsJson) {
    this.levelDefinitions = levelsJson;
  }

  parseLevelDef(filename, episodeNum) {
    const raw = this.levelDefinitions[filename] || this.levelDefinitions['levelsb.dat'] || '';
    const lines = raw.split(/\r?\n/).map(l => l.trim()).filter(l => l.length > 0);

    let idx = 0;
    const episodes = [];

    while (idx < lines.length) {
      if (idx + 28 > lines.length) break;
      const response = lines[idx++];
      const title = lines[idx++];
      const gametype = parseInt(lines[idx++]);
      const beholders = parseInt(lines[idx++]);
      const bspeed = parseInt(lines[idx++]);
      const deathorb = parseInt(lines[idx++]);
      const dspeed = parseInt(lines[idx++]);
      const skeletons = parseInt(lines[idx++]);
      const sspeed = parseInt(lines[idx++]);
      const mouther = parseInt(lines[idx++]);
      const mspeed = parseInt(lines[idx++]);
      const ogre = parseInt(lines[idx++]);
      const ospeed = parseInt(lines[idx++]);
      const fighter = parseInt(lines[idx++]);
      const fteam = parseInt(lines[idx++]);
      const fspeed = parseInt(lines[idx++]);
      const cleric = parseInt(lines[idx++]);
      const cteam = parseInt(lines[idx++]);
      const cspeed = parseInt(lines[idx++]);
      const thief = parseInt(lines[idx++]);
      const tteam = parseInt(lines[idx++]);
      const tspeed = parseInt(lines[idx++]);
      const wizard = parseInt(lines[idx++]);
      const wteam = parseInt(lines[idx++]);
      const wspeed = parseInt(lines[idx++]);
      const dungeonx = parseInt(lines[idx++]);
      const dungeony = parseInt(lines[idx++]);
      const dungeonlvl = parseInt(lines[idx++]);
      const pits = parseInt(lines[idx++]);
      const maxpit = parseInt(lines[idx++]);

      episodes.push({
        response, title, gametype,
        beholders, bspeed, deathorb, dspeed,
        skeletons, sspeed, mouther, mspeed, ogre, ospeed,
        fighter, fteam, fspeed, cleric, cteam, cspeed,
        thief, tteam, tspeed, wizard, wteam, wspeed,
        dungeonx, dungeony, dungeonlvl, pits, maxpit
      });
    }

    const clampedIdx = Math.min(Math.max(0, episodeNum - 1), episodes.length - 1);
    this.currentEpisodeDef = episodes[clampedIdx] || {
      response: "Find the -p and return it home",
      title: "Episode 1",
      gametype: 1,
      beholders: 1, bspeed: 2,
      deathorb: 0, dspeed: 1,
      skeletons: 4, sspeed: 2,
      mouther: 2, mspeed: 2,
      ogre: 1, ospeed: 2,
      fighter: 0, cleric: 0, thief: 0, wizard: 0,
      dungeonx: 14, dungeony: 14, dungeonlvl: 3,
      pits: 1, maxpit: 6
    };
    return this.currentEpisodeDef;
  }

  randomNum(max) {
    return Math.floor(Math.random() * max);
  }

  clearDungeon() {
    this.dungeon = {};
    for (let x = 0; x <= this.DUNGEONX + 2; x++) {
      this.dungeon[x] = {};
      for (let y = 0; y <= this.DUNGEONY + 2; y++) {
        this.dungeon[x][y] = {};
        for (let lvl = 0; lvl <= this.maxlevel + 2; lvl++) {
          this.dungeon[x][y][lvl] = {
            type: 0,
            explored: 1,
            item: 0,
            image: 0,
            mark: 0,
            order: 0
          };
        }
      }
    }
  }

  initDungeon() {
    for (let lvl = 0; lvl <= this.maxlevel; lvl++) {
      this.dimage[lvl] = this.randomNum(2) === 1 ? 1 : 2;
    }

    for (let lvl = 1; lvl <= this.maxlevel; lvl++) {
      for (let y = 0; y < this.DUNGEONY; y++) {
        for (let x = 0; x < this.DUNGEONX; x++) {
          this.dungeon[x][y][lvl].type = 'f';
          this.dungeon[x][y][lvl].item = 0;
        }
      }

      let work1 = 8;
      for (let y = 1; y < 8; y++) {
        for (let x = 1; x < work1; x++) {
          this.dungeon[x][y][lvl].type = 'u';
        }
        work1--;
      }

      let work2 = 10;
      work1 = 25;
      for (let y = 1; y < 13; y++) {
        for (let x = work2; x < work1; x++) {
          if (this.dungeon[x] && this.dungeon[x][y]) {
            this.dungeon[x][y][lvl].type = 'u';
          }
        }
        work2++;
      }
    }
  }

  generateDungeon() {
    const sx = Math.floor(this.DUNGEONX / 2);
    const sy = Math.floor(this.DUNGEONY / 2);

    let lvlnum = 0;
    let stopit = 0;

    while (lvlnum < this.maxlevel) {
      lvlnum++;
      stopit++;
      if (stopit >= (this.DUNGEONX * this.DUNGEONY * this.maxlevel) * 3) {
        lvlnum = 1;
        break;
      }

      for (let y = 0; y <= this.DUNGEONY + 1; y++) {
        for (let x = 0; x <= this.DUNGEONX + 1; x++) {
          if (this.dungeon[x] && this.dungeon[x][y]) {
            this.dungeon[x][y][lvlnum].type = 0;
            this.dungeon[x][y][lvlnum].item = 0;
          }
        }
      }

      for (let jump = 0; jump < 4; jump++) {
        let x = sx;
        let y = sy;
        this.dungeon[x][y][lvlnum].type = 'f';

        for (let i = 0; i < 75; i++) {
          let xpos = 0;
          let ypos = 0;
          const way = this.randomNum(4);
          if (way === 0) xpos = -1;
          else if (way === 1) xpos = 1;
          else if (way === 2) ypos = -1;
          else if (way === 3) ypos = 1;

          const nx = x + xpos;
          const ny = y + ypos;

          if (nx > 0 && nx < this.DUNGEONX && ny > 0 && ny < this.DUNGEONY) {
            if (this.dungeon[nx][ny][lvlnum].type !== 'u') {
              this.dungeon[nx][ny][lvlnum].type = 'f';
              x = nx;
              y = ny;
            }
          }
        }
      }

      let floorCount = 0;
      for (let y = 1; y < this.DUNGEONY; y++) {
        for (let x = 1; x < this.DUNGEONX; x++) {
          if (this.dungeon[x][y][lvlnum].type === 'f') floorCount++;
        }
      }

      if (floorCount < 25) {
        for (let cy = Math.max(1, sy - 2); cy <= Math.min(this.DUNGEONY - 1, sy + 2); cy++) {
          for (let cx = Math.max(1, sx - 2); cx <= Math.min(this.DUNGEONX - 1, sx + 2); cx++) {
            this.dungeon[cx][cy][lvlnum].type = 'f';
          }
        }
      }

      if (lvlnum > 1) {
        let stairsPlaced = false;
        let tries = 0;
        while (!stairsPlaced && tries < 300) {
          tries++;
          const rx = this.randomNum(this.DUNGEONX - 2) + 1;
          const ry = this.randomNum(this.DUNGEONY - 2) + 1;
          if (this.dungeon[rx][ry][lvlnum - 1].type === 'f' && this.dungeon[rx][ry][lvlnum].type === 'f') {
            this.dungeon[rx][ry][lvlnum - 1].type = 's';
            this.dungeon[rx][ry][lvlnum - 1].item = 0;
            this.dungeon[rx][ry][lvlnum].type = 's';
            this.dungeon[rx][ry][lvlnum].item = 1;
            stairsPlaced = true;
          }
        }

        if (!stairsPlaced) {
          this.dungeon[sx][sy][lvlnum - 1].type = 's';
          this.dungeon[sx][sy][lvlnum - 1].item = 0;
          this.dungeon[sx][sy][lvlnum].type = 's';
          this.dungeon[sx][sy][lvlnum].item = 1;
        }
      }
    }
  }

  assignTreasures(gametype) {
    let placed = false;
    let tries = 0;
    while (!placed && tries < 500) {
      tries++;
      const rx = this.randomNum(this.DUNGEONX - 1) + 1;
      const ry = this.randomNum(this.DUNGEONY - 1) + 1;
      const lvl = 1;
      if (this.dungeon[rx][ry][lvl].type === 'f' && this.dungeon[rx][ry][lvl].item === 0) {
        this.treasurex = rx;
        this.treasurey = ry;
        this.treasurelvl = lvl;
        this.foundtreasure = 0;
        placed = true;
      }
    }

    placed = false;
    tries = 0;
    while (!placed && tries < 500) {
      tries++;
      const rx = this.randomNum(this.DUNGEONX - 1) + 1;
      const ry = this.randomNum(this.DUNGEONY - 1) + 1;
      const lvl = this.maxlevel;
      if (this.dungeon[rx][ry][lvl].type === 'f' && this.dungeon[rx][ry][lvl].item === 0) {
        this.treasurex2 = rx;
        this.treasurey2 = ry;
        this.treasurelvl2 = lvl;
        this.foundtreasure2 = 0;
        placed = true;
      }
    }
  }

  assignItems(count) {
    for (let i = 0; i < count; i++) {
      let placed = false;
      let tries = 0;
      while (!placed && tries < 200) {
        tries++;
        const rx = this.randomNum(this.DUNGEONX - 1) + 1;
        const ry = this.randomNum(this.DUNGEONY - 1) + 1;
        const lvl = this.randomNum(this.maxlevel) + 1;
        if (this.dungeon[rx][ry][lvl].type === 'f' && this.dungeon[rx][ry][lvl].item === 0) {
          const r = this.randomNum(5);
          if (r === 0) this.dungeon[rx][ry][lvl].item = 'a';
          else if (r === 1) this.dungeon[rx][ry][lvl].item = 'r';
          else if (r === 2) this.dungeon[rx][ry][lvl].item = 's';
          else this.dungeon[rx][ry][lvl].item = 'e';
          placed = true;
        }
      }
    }

    for (let lvl = 1; lvl <= this.maxlevel; lvl++) {
      for (let k = 0; k < 2; k++) {
        let placed = false;
        let tries = 0;
        while (!placed && tries < 100) {
          tries++;
          const rx = this.randomNum(this.DUNGEONX - 1) + 1;
          const ry = this.randomNum(this.DUNGEONY - 1) + 1;
          if (this.dungeon[rx][ry][lvl].type === 'f' && this.dungeon[rx][ry][lvl].item === 0) {
            this.dungeon[rx][ry][lvl].item = 'r';
            placed = true;
          }
        }
      }
    }
  }

  assignPits(maxPits) {
    this.pits = [];
    if (!maxPits || maxPits <= 0) return;

    for (let count = 0; count < maxPits; count++) {
      let placed = false;
      let tries = 0;
      while (!placed && tries < 200) {
        tries++;
        const rx = this.randomNum(this.DUNGEONX - 1) + 1;
        const ry = this.randomNum(this.DUNGEONY - 1) + 1;
        const lvl = this.randomNum(this.maxlevel - 1) + 1;
        if (this.dungeon[rx][ry][lvl + 1].type === 'f' &&
            this.dungeon[rx][ry][lvl].type === 'f' &&
            this.dungeon[rx][ry][lvl].item === 0) {
          this.pits.push({
            x: rx,
            y: ry,
            lvl: lvl,
            frame: 1
          });
          placed = true;
        }
      }
    }
  }

  setTileTypes() {
    this.tile_type = [];
    let cur = 1;
    for (let i = 0; i < 40; i++) {
      this.tile_type[i] = cur;
      cur = (cur % 3) + 1;
    }
  }

  buildNewLevel(classFileName, episodeNumber) {
    const ep = this.parseLevelDef(classFileName, episodeNumber);
    this.DUNGEONX = ep.dungeonx || 14;
    this.DUNGEONY = ep.dungeony || 14;
    this.maxlevel = ep.dungeonlvl || 3;

    this.clearDungeon();
    this.setTileTypes();
    this.initDungeon();
    this.generateDungeon();
    this.assignTreasures(ep.gametype);
    this.assignItems(12 + this.maxlevel * 2);
    if (ep.pits) {
      this.assignPits(ep.maxpit || 6);
    }

    return ep;
  }

  findSpawnLocation(level = 1) {
    for (let tries = 0; tries < 500; tries++) {
      const rx = this.randomNum(this.DUNGEONX - 1) + 1;
      const ry = this.randomNum(this.DUNGEONY - 1) + 1;
      if (this.dungeon[rx][ry][level].type === 'f' && this.dungeon[rx][ry][level].item === 0) {
        return { x: rx, y: ry, lvl: level };
      }
    }
    const sx = Math.floor(this.DUNGEONX / 2);
    const sy = Math.floor(this.DUNGEONY / 2);
    return { x: sx, y: sy, lvl: level };
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { DungeonSystem };
}
