/**
 * Murk dungeon model: episode parsing, procedural generation and grid queries.
 * Generation follows the original C++ (MurkInit.cpp): winding single-width tunnels carved by
 * random walks that never complete a 2x2 block, stacked floors joined by ladders.
 */

const VOID_CELL = Object.freeze({ type: 0, item: 0, explored: 0, pad: -1, stair: null });

const DIR = { NW: 1, NE: 2, SW: 3, SE: 4 };
// Grid step per direction: 1 = x-1, 2 = y-1, 3 = y+1, 4 = x+1 (matches the original sprite sheet).
const DIR_VEC = { 1: [-1, 0], 2: [0, -1], 3: [0, 1], 4: [1, 0] };
const DIRS = [1, 2, 3, 4];

function dirFromDelta(dx, dy) {
  if (Math.abs(dx) >= Math.abs(dy)) return dx < 0 ? DIR.NW : DIR.SE;
  return dy < 0 ? DIR.NE : DIR.SW;
}

class DungeonSystem {
  constructor() {
    this.N = 14;
    this.maxlevel = 3;
    this.levels = [];
    this.pits = [];
    this.relics = [];
    this.tileType = [];
    this.levelDefinitions = {};
    this.episodeCache = {};
    this.currentEpisodeDef = null;
  }

  loadLevelDefs(json) {
    this.levelDefinitions = json || {};
    this.episodeCache = {};
  }

  parseEpisodes(filename) {
    if (this.episodeCache[filename]) return this.episodeCache[filename];
    const raw = this.levelDefinitions[filename] || this.levelDefinitions['levelsb.dat'] || '';
    const lines = raw.split(/\r?\n/);
    const episodes = [];
    const num = (i) => parseInt(lines[i], 10) || 0;

    for (let i = 0; i + 30 <= lines.length; i += 30) {
      const b = i + 2;
      episodes.push({
        response: lines[i].trim(),
        title: lines[i + 1].trim(),
        gametype: num(b),
        beholders: num(b + 1), bspeed: num(b + 2),
        deathorb: num(b + 3), dspeed: num(b + 4),
        skeletons: num(b + 5), sspeed: num(b + 6),
        mouther: num(b + 7), mspeed: num(b + 8),
        ogre: num(b + 9), ospeed: num(b + 10),
        fighter: num(b + 11), fteam: num(b + 12), fspeed: num(b + 13),
        cleric: num(b + 14), cteam: num(b + 15), cspeed: num(b + 16),
        thief: num(b + 17), tteam: num(b + 18), tspeed: num(b + 19),
        wizard: num(b + 20), wteam: num(b + 21), wspeed: num(b + 22),
        dungeonx: num(b + 23), dungeony: num(b + 24), dungeonlvl: num(b + 25),
        pits: num(b + 26), maxpit: num(b + 27)
      });
    }
    if (episodes.length === 0) {
      episodes.push({
        response: 'Find the -p and return it to -k .', title: 'Episode 1', gametype: 1,
        beholders: 1, bspeed: 2, deathorb: 0, dspeed: 1, skeletons: 4, sspeed: 2,
        mouther: 2, mspeed: 2, ogre: 1, ospeed: 2,
        fighter: 0, fteam: 0, fspeed: 1, cleric: 0, cteam: 0, cspeed: 1,
        thief: 0, tteam: 0, tspeed: 1, wizard: 0, wteam: 0, wspeed: 1,
        dungeonx: 14, dungeony: 14, dungeonlvl: 3, pits: 1, maxpit: 6
      });
    }
    this.episodeCache[filename] = episodes;
    return episodes;
  }

  getEpisode(filename, episodeNum) {
    const eps = this.parseEpisodes(filename);
    return eps[(Math.max(1, episodeNum) - 1) % eps.length];
  }

  randomNum(max) {
    return Math.floor(Math.random() * max);
  }

  // ---------- grid access ----------

  inBounds(x, y) {
    return x >= 0 && y >= 0 && x <= this.N + 1 && y <= this.N + 1;
  }

  cell(x, y, lvl) {
    if (!this.inBounds(x, y)) return VOID_CELL;
    const level = this.levels[lvl];
    return level ? level[y * (this.N + 2) + x] : VOID_CELL;
  }

  walkable(x, y, lvl) {
    const t = this.cell(x, y, lvl).type;
    return t === 'f' || t === 's';
  }

  forEachCell(lvl, fn) {
    for (let y = 1; y <= this.N; y++) {
      for (let x = 1; x <= this.N; x++) fn(this.cell(x, y, lvl), x, y);
    }
  }

  // ---------- generation ----------

  build(ep) {
    this.currentEpisodeDef = ep;
    this.N = Math.max(8, Math.max(ep.dungeonx || 14, ep.dungeony || 14));
    this.maxlevel = Math.max(2, ep.dungeonlvl || 3);
    this.pits = [];
    this.relics = [];
    this.tileType = [];
    for (let l = 1; l <= this.maxlevel; l++) this.tileType[l] = ((l - 1) % 3) + 1;

    this.levels = [];
    for (let l = 1; l <= this.maxlevel; l++) {
      this.levels[l] = this.carveLevel(l);
      if (l > 1) this.linkLevels(l - 1, l);
    }

    this.assignRelics(ep.gametype);
    this.assignItems(8 + this.maxlevel * 2);
    return ep;
  }

  newLevelGrid() {
    const size = (this.N + 2) * (this.N + 2);
    const grid = new Array(size);
    for (let i = 0; i < size; i++) {
      grid[i] = { type: 0, item: 0, explored: 0, pad: -1, stair: null };
    }
    return grid;
  }

  carveLevel(lvl) {
    const N = this.N;
    const W = N + 2;
    const center = Math.floor(N / 2);
    const jumps = 4;
    const steps = Math.round(1.3 * 75 * (N * N) / 196);
    let grid;

    for (let attempt = 0; attempt < 30; attempt++) {
      grid = this.newLevelGrid();

      const k = Math.floor(N * 0.35);
      for (let y = 1; y <= N; y++) {
        for (let x = 1; x <= N; x++) {
          if (x + y <= k || x + y >= 2 * N - k || x - y >= N - k || y - x >= N - k) {
            grid[y * W + x].type = 'u';
          }
        }
      }

      for (let j = 0; j < jumps; j++) {
        let x = center;
        let y = center;
        grid[y * W + x].type = 'f';
        for (let i = 0; i < steps; i++) {
          const v = DIR_VEC[1 + this.randomNum(4)];
          const nx = x + v[0];
          const ny = y + v[1];
          if (nx <= 0 || nx >= N || ny <= 0 || ny >= N) continue;
          const t = grid[ny * W + nx].type;
          if (t === 'u' || t === 's') continue;
          if (this.randomNum(30) !== 1 && this.completesBlock(grid, nx, ny)) continue;
          grid[ny * W + nx].type = 'f';
          x = nx;
          y = ny;
        }
      }

      let floors = 0;
      for (let i = 0; i < grid.length; i++) if (grid[i].type === 'f') floors++;
      if (floors >= Math.max(40, N * N * 0.25)) break;
    }
    return grid;
  }

  completesBlock(grid, x, y) {
    const W = this.N + 2;
    const f = (ax, ay) => grid[ay * W + ax].type === 'f';
    for (const sx of [-1, 1]) {
      for (const sy of [-1, 1]) {
        if (f(x + sx, y) && f(x, y + sy) && f(x + sx, y + sy)) return true;
      }
    }
    return false;
  }

  linkLevels(upper, lower) {
    const candidates = [];
    this.forEachCell(upper, (c, x, y) => {
      if (c.type === 'f' && this.cell(x, y, lower).type === 'f') candidates.push({ x, y });
    });
    for (let i = candidates.length - 1; i > 0; i--) {
      const j = this.randomNum(i + 1);
      [candidates[i], candidates[j]] = [candidates[j], candidates[i]];
    }

    const placed = [];
    for (const c of candidates) {
      if (placed.length >= 3) break;
      if (placed.some(p => Math.abs(p.x - c.x) + Math.abs(p.y - c.y) < 4)) continue;
      placed.push(c);
    }
    if (placed.length === 0 && candidates.length) placed.push(candidates[0]);

    for (const p of placed) {
      const a = this.cell(p.x, p.y, upper);
      const b = this.cell(p.x, p.y, lower);
      a.type = 's'; a.stair = 'down'; a.item = 0;
      b.type = 's'; b.stair = 'up'; b.item = 0;
    }
  }

  isFreeFloor(x, y, lvl) {
    const c = this.cell(x, y, lvl);
    return c.type === 'f' && c.item === 0 && c.pad < 0;
  }

  randomFloor(lvl, accept) {
    const N = this.N;
    for (let tries = 0; tries < 1000; tries++) {
      const x = this.randomNum(N) + 1;
      const y = this.randomNum(N) + 1;
      if (this.isFreeFloor(x, y, lvl) && (!accept || accept(x, y))) return { x, y, lvl };
    }
    return null;
  }

  assignRelics(gametype) {
    // Relic 1 (red chest) sits on the first floor, relic 2 (blue chest) is hidden deeper.
    if (gametype !== 1) {
      const p = this.randomFloor(1);
      if (p) this.relics.push({ id: 1, x: p.x, y: p.y, lvl: 1, carrier: null });
    }
    const deepLvl = this.randomNum(this.maxlevel - 1) + 2;
    const p2 = this.randomFloor(deepLvl) || this.randomFloor(this.maxlevel);
    if (p2) this.relics.push({ id: 2, x: p2.x, y: p2.y, lvl: p2.lvl, carrier: null });
  }

  relic(id) {
    return this.relics.find(r => r.id === id) || null;
  }

  assignItems(count) {
    const pick = () => {
      const r = this.randomNum(5);
      if (r === 0) return 'a';
      if (r === 2) return 's';
      return 'e';
    };
    for (let i = 0; i < count; i++) {
      const p = this.randomFloor(this.randomNum(this.maxlevel) + 1);
      if (p) this.cell(p.x, p.y, p.lvl).item = pick();
    }
    for (let i = 0; i < 2; i++) {
      const p = this.randomFloor(this.randomNum(this.maxlevel) + 1);
      if (p) this.cell(p.x, p.y, p.lvl).item = 'r';
    }
  }

  // ---------- queries ----------

  // Marks nearby tunnel cells (reachable within `depth` steps) as explored; returns true if anything new appeared.
  reveal(lvl, x, y, depth = 4) {
    let changed = false;
    const W = this.N + 2;
    const seen = new Set([y * W + x]);
    let frontier = [[x, y]];
    const start = this.cell(x, y, lvl);
    if (start.type !== 0 && !start.explored) { start.explored = 1; changed = true; }

    for (let d = 0; d < depth && frontier.length; d++) {
      const next = [];
      for (const [cx, cy] of frontier) {
        for (const dir of DIRS) {
          const nx = cx + DIR_VEC[dir][0];
          const ny = cy + DIR_VEC[dir][1];
          const key = ny * W + nx;
          if (seen.has(key) || !this.walkable(nx, ny, lvl)) continue;
          seen.add(key);
          const c = this.cell(nx, ny, lvl);
          if (!c.explored) { c.explored = 1; changed = true; }
          next.push([nx, ny]);
        }
      }
      frontier = next;
    }
    return changed;
  }

  // Shortest walkable route from (sx,sy) to (tx,ty); `blocked(x,y)` may veto cells. Returns the steps (excluding start).
  findPath(lvl, sx, sy, tx, ty, blocked, maxNodes = 900) {
    if (!this.walkable(tx, ty, lvl)) return null;
    const W = this.N + 2;
    const prev = new Map();
    const start = sy * W + sx;
    prev.set(start, -1);
    const queue = [[sx, sy]];
    let head = 0;
    while (head < queue.length && head < maxNodes) {
      const [cx, cy] = queue[head++];
      if (cx === tx && cy === ty) {
        const path = [];
        let k = cy * W + cx;
        while (k !== start) {
          path.push({ x: k % W, y: Math.floor(k / W) });
          k = prev.get(k);
        }
        return path.reverse();
      }
      for (const dir of DIRS) {
        const nx = cx + DIR_VEC[dir][0];
        const ny = cy + DIR_VEC[dir][1];
        const key = ny * W + nx;
        if (prev.has(key) || !this.walkable(nx, ny, lvl)) continue;
        if (blocked && !(nx === tx && ny === ty) && blocked(nx, ny)) continue;
        prev.set(key, cy * W + cx);
        queue.push([nx, ny]);
      }
    }
    return null;
  }

  // Walks from a cell along a direction until a wall; returns the cells passed (nearest first).
  castLine(lvl, x, y, dir, maxDist) {
    const cells = [];
    let cx = x;
    let cy = y;
    for (let i = 0; i < maxDist; i++) {
      cx += DIR_VEC[dir][0];
      cy += DIR_VEC[dir][1];
      if (!this.walkable(cx, cy, lvl)) break;
      cells.push({ x: cx, y: cy });
    }
    return cells;
  }

  findStairs(lvl, kind) {
    const out = [];
    this.forEachCell(lvl, (c, x, y) => {
      if (c.type === 's' && c.stair === kind) out.push({ x, y });
    });
    return out;
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { DungeonSystem, DIR, DIR_VEC, DIRS, VOID_CELL, dirFromDelta };
}
