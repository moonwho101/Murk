/**
 * Murk renderer.
 *
 * The original art is pre-rendered isometric sprites, so the scene is drawn as an orthographic,
 * pixel-aligned sprite world in WebGL (three.js). Sprites are painter-sorted exactly like the DirectDraw
 * original, then lit by a flickering torch shader, dynamic glow, particles and screen shake.
 */

const TILE_W = 112;
const TILE_H = 97;
// Tile art origin (top-left) for a grid cell, and the point on the floor where creatures stand.
const ISO = {
  originX: (x, y) => 49 * x - 48 * y,
  originY: (x, y) => 25 * x + 24 * y,
  footX: 51,
  footY: 68,
  spriteDX: -31,
  spriteDY: -58
};

const TILE_RECTS = {
  1: [0, 98], 2: [113, 98], 3: [226, 98], 4: [339, 98], 12: [452, 98], 23: [565, 98], 34: [678, 98],
  14: [0, 196], 13: [113, 196], 24: [226, 196], 123: [339, 196], 234: [452, 196], 134: [565, 196], 124: [678, 196],
  1234: [0, 0]
};

const ITEM_ART = {
  e: { rect: [226, 490, 112, 97], dy: 20 },
  s: { rect: [565, 0, 113, 97], dy: 0 },
  r: { rect: [678, 0, 113, 97], dy: 0 },
  a: { rect: [450, 0, 113, 97], dy: 0 }
};

// Home pad art per class pad index, and the monsters' lair marker.
const PAD_ART = [
  [113, 294, 112, 97], // green
  [337, 294, 112, 97], // blue
  [449, 294, 112, 97], // dark green
  [561, 294, 112, 97], // cyan
  [224, 294, 113, 97] // lair
];

const PIT_ART = [[452, 490, 113, 98], [565, 490, 113, 98], [678, 490, 113, 98]];

const SPRITE_VERT = `
uniform vec4 uRect;
varying vec2 vUv;
varying vec2 vWorld;
void main() {
  vUv = uRect.xy + uv * uRect.zw;
  vec4 w = modelMatrix * vec4(position, 1.0);
  vWorld = w.xy;
  gl_Position = projectionMatrix * viewMatrix * w;
}`;

const SPRITE_FRAG = `
uniform sampler2D uMap;
uniform vec3 uTint;
uniform float uAlpha;
uniform float uAmbient;
uniform float uFlash;
uniform vec2 uLight;
uniform float uRadius;
uniform float uTime;
varying vec2 vUv;
varying vec2 vWorld;
void main() {
  vec4 c = texture2D(uMap, vUv);
  if (c.a < 0.08) discard;
  vec2 d = vWorld - uLight;
  d.y *= 2.0;
  float r = uRadius * (1.0 + 0.035 * sin(uTime * 7.0) + 0.02 * sin(uTime * 13.0 + 1.3));
  float f = clamp(1.0 - length(d) / r, 0.0, 1.0);
  f = f * f * (3.0 - 2.0 * f);
  vec3 cool = vec3(0.62, 0.68, 1.0) * uAmbient;
  vec3 warm = vec3(1.12, 0.96, 0.78);
  vec3 lit = mix(cool, warm, f);
  vec3 rgb = c.rgb * lit * uTint + uFlash * vec3(0.9, 0.12, 0.1);
  gl_FragColor = vec4(rgb, c.a * uAlpha);
}`;

class MurkRenderer {
  constructor(container) {
    this.container = container;
    this.textures = {};
    this.materialCache = new Map();
    this.views = new Map();
    this.cellViews = [];
    this.relicViews = [];
    this.pitViews = [];
    this.projViews = [];
    this.floaters = [];
    this.particles = [];
    this.time = 0;
    this.scale = 2;
    this.userZoom = 0; // 0 = automatic
    this.cam = { x: 0, y: 0 };
    this.camSnap = true;
    this.shake = 0;
    this.lightRadius = 300;

    this.initThree();
  }

  initThree() {
    THREE.ColorManagement.enabled = false;
    this.scene = new THREE.Scene();
    this.camera = new THREE.OrthographicCamera(-400, 400, 300, -300, -100, 100);
    this.camera.position.z = 10;

    this.renderer = new THREE.WebGLRenderer({ antialias: false, alpha: false, powerPreference: 'high-performance' });
    this.renderer.outputColorSpace = THREE.LinearSRGBColorSpace;
    this.renderer.setClearColor(0x000000, 1);
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    this.container.appendChild(this.renderer.domElement);

    this.shared = {
      uLight: { value: new THREE.Vector2(0, 0) },
      uRadius: { value: this.lightRadius },
      uTime: { value: 0 }
    };

    this.levelGroup = new THREE.Group();
    this.entityGroup = new THREE.Group();
    this.fxGroup = new THREE.Group();
    this.scene.add(this.levelGroup, this.entityGroup, this.fxGroup);

    this.quad = new THREE.PlaneGeometry(1, 1);
    this.quad.translate(0.5, -0.5, 0);

    this.glowTex = this.makeGlowTexture(false);
    this.ringTex = this.makeGlowTexture(true);
    this.shadowTex = this.makeShadowTexture();

    this.onResize();
    window.addEventListener('resize', () => this.onResize());
  }

  makeGlowTexture(ring) {
    const c = document.createElement('canvas');
    c.width = c.height = 64;
    const g = c.getContext('2d');
    const grad = g.createRadialGradient(32, 32, 0, 32, 32, 32);
    if (ring) {
      grad.addColorStop(0, 'rgba(255,255,255,0.05)');
      grad.addColorStop(0.7, 'rgba(255,255,255,0.15)');
      grad.addColorStop(0.9, 'rgba(255,255,255,0.9)');
      grad.addColorStop(1, 'rgba(255,255,255,0)');
    } else {
      grad.addColorStop(0, 'rgba(255,255,255,1)');
      grad.addColorStop(0.35, 'rgba(255,255,255,0.45)');
      grad.addColorStop(1, 'rgba(255,255,255,0)');
    }
    g.fillStyle = grad;
    g.fillRect(0, 0, 64, 64);
    return new THREE.CanvasTexture(c);
  }

  makeShadowTexture() {
    const c = document.createElement('canvas');
    c.width = 64;
    c.height = 32;
    const g = c.getContext('2d');
    g.translate(32, 16);
    g.scale(1, 0.5);
    const grad = g.createRadialGradient(0, 0, 0, 0, 0, 30);
    grad.addColorStop(0, 'rgba(0,0,0,0.75)');
    grad.addColorStop(0.7, 'rgba(0,0,0,0.45)');
    grad.addColorStop(1, 'rgba(0,0,0,0)');
    g.fillStyle = grad;
    g.fillRect(-32, -32, 64, 64);
    return new THREE.CanvasTexture(c);
  }

  onResize() {
    const w = this.container.clientWidth || window.innerWidth;
    const h = this.container.clientHeight || window.innerHeight;
    this.width = w;
    this.height = h;
    this.renderer.setSize(w, h);
    this.updateScale();
  }

  updateScale() {
    const auto = Math.max(1, Math.round(this.height / 420));
    this.scale = this.userZoom || auto;
    const halfW = this.width / (2 * this.scale);
    const halfH = this.height / (2 * this.scale);
    this.camera.left = -halfW;
    this.camera.right = halfW;
    this.camera.top = halfH;
    this.camera.bottom = -halfH;
    this.camera.updateProjectionMatrix();
    this.lightRadius = Math.max(260, Math.min(420, halfH * 1.15));
    this.shared.uRadius.value = this.lightRadius;
  }

  zoomBy(step) {
    const levels = [1, 1.5, 2, 3, 4];
    const cur = levels.reduce((best, v) => (Math.abs(v - this.scale) < Math.abs(best - this.scale) ? v : best), levels[0]);
    const idx = Math.max(0, Math.min(levels.length - 1, levels.indexOf(cur) + step));
    this.userZoom = levels[idx];
    this.updateScale();
  }

  async loadTextures() {
    const loader = new THREE.TextureLoader();
    const load = (url) => new Promise((resolve, reject) => {
      loader.load(url, (tex) => {
        tex.magFilter = THREE.NearestFilter;
        tex.minFilter = THREE.NearestFilter;
        tex.generateMipmaps = false;
        tex.colorSpace = THREE.NoColorSpace;
        resolve(tex);
      }, undefined, reject);
    });
    const [creatures, parts, floor2, floor3] = await Promise.all([
      load('assets/p256.png'), load('assets/dungeon.png'), load('assets/dungeon2.png'), load('assets/dungeon3.png')
    ]);
    this.textures = { creatures, parts, floor1: parts, floor2, floor3 };
  }

  // ---------- projection ----------

  gridToWorld(x, y) {
    return { x: ISO.originX(x, y) + ISO.footX, y: -(ISO.originY(x, y) + ISO.footY) };
  }

  worldToGrid(wx, wy) {
    const u = wx - ISO.footX;
    const v = -wy - ISO.footY;
    return { x: (24 * u + 48 * v) / 2376, y: (-25 * u + 49 * v) / 2376 };
  }

  screenToGrid(clientX, clientY) {
    const rect = this.renderer.domElement.getBoundingClientRect();
    const wx = this.cam.x + (clientX - rect.left - rect.width / 2) / this.scale;
    const wy = this.cam.y - (clientY - rect.top - rect.height / 2) / this.scale;
    const g = this.worldToGrid(wx, wy);
    return { x: Math.round(g.x), y: Math.round(g.y), fx: g.x, fy: g.y };
  }

  // ---------- sprite helpers ----------

  getMaterial(sheet, rect, opts = {}) {
    const key = `${sheet}:${rect.join(',')}:${opts.ambient ?? 0.3}`;
    let mat = this.materialCache.get(key);
    if (!mat) {
      mat = this.makeMaterial(sheet, rect, opts);
      this.materialCache.set(key, mat);
    }
    return mat;
  }

  makeMaterial(sheet, rect, opts = {}) {
    const tex = this.textures[sheet];
    const W = tex.image.width;
    const H = tex.image.height;
    const eps = 0.02;
    const [x, y, w, h] = rect;
    const uRect = new THREE.Vector4(
      (x + eps) / W,
      1 - (y + h - eps) / H,
      (w - 2 * eps) / W,
      (h - 2 * eps) / H
    );
    return new THREE.ShaderMaterial({
      uniforms: {
        uMap: { value: tex },
        uRect: { value: uRect },
        uTint: { value: new THREE.Vector3(1, 1, 1) },
        uAlpha: { value: 1 },
        uAmbient: { value: opts.ambient ?? 0.3 },
        uFlash: { value: 0 },
        uLight: this.shared.uLight,
        uRadius: this.shared.uRadius,
        uTime: this.shared.uTime
      },
      vertexShader: SPRITE_VERT,
      fragmentShader: SPRITE_FRAG,
      transparent: true,
      depthTest: false,
      depthWrite: false
    });
  }

  setRect(mat, rect) {
    const tex = mat.uniforms.uMap.value;
    const W = tex.image.width;
    const H = tex.image.height;
    const eps = 0.02;
    const [x, y, w, h] = rect;
    mat.uniforms.uRect.value.set((x + eps) / W, 1 - (y + h - eps) / H, (w - 2 * eps) / W, (h - 2 * eps) / H);
  }

  makeMesh(material, w, h) {
    const mesh = new THREE.Mesh(this.quad, material);
    mesh.scale.set(w, h, 1);
    mesh.matrixAutoUpdate = true;
    return mesh;
  }

  placeTopLeft(mesh, px, py, order) {
    mesh.position.set(px, -py, 0);
    mesh.renderOrder = order;
  }

  orderKey(ds, x, y, layer) {
    return (Math.round(y) * (ds.N + 3) + Math.round(x)) * 10 + layer;
  }

  // ---------- level construction ----------

  clearGroup(group) {
    while (group.children.length) group.remove(group.children[0]);
  }

  buildLevel(ds, lvl) {
    this.clearGroup(this.levelGroup);
    this.cellViews = [];
    this.relicViews = [];
    this.pitViews = [];

    const sheet = `floor${ds.tileType[lvl] || 1}`;
    const W = ds.N + 2;
    ds.forEachCell(lvl, (cell, x, y) => {
      if (cell.type !== 'f' && cell.type !== 's') return;
      const f = (dx, dy) => (ds.walkable(x + dx, y + dy, lvl) ? 1 : 0);
      let code = '';
      if (f(-1, 0)) code += '1';
      if (f(0, -1)) code += '2';
      if (f(1, 0)) code += '3';
      if (f(0, 1)) code += '4';
      const key = parseInt(code, 10) || 1234;
      const [rx, ry] = TILE_RECTS[key] || TILE_RECTS[1234];

      const ox = ISO.originX(x, y);
      const oy = ISO.originY(x, y);
      const view = { x, y, tile: null, stair: null, pad: null, item: null, itemKey: null, padKey: null, baseY: oy };

      view.tile = this.makeMesh(this.getMaterial(sheet, [rx, ry, TILE_W, TILE_H], { ambient: 0.34 }), TILE_W, TILE_H);
      this.placeTopLeft(view.tile, ox, oy, this.orderKey(ds, x, y, 0));
      this.levelGroup.add(view.tile);

      if (cell.type === 's') {
        const rect = cell.stair === 'up' ? [224, 0, 112, 97] : [112, 0, 112, 97];
        view.stair = this.makeMesh(this.getMaterial('parts', rect, { ambient: 0.4 }), 112, 97);
        this.placeTopLeft(view.stair, ox, oy, this.orderKey(ds, x, y, 1));
        this.levelGroup.add(view.stair);
      }
      this.cellViews[y * W + x] = view;
    });

    for (let i = 0; i < 8; i++) {
      const mesh = this.makeMesh(this.getMaterial('parts', PIT_ART[0], { ambient: 0.4 }), 113, 98);
      mesh.visible = false;
      this.levelGroup.add(mesh);
      this.pitViews.push(mesh);
    }

    ds.relics.forEach((relic) => {
      const rect = relic.id === 1 ? [0, 294, 112, 97] : [678, 294, 112, 97];
      const mesh = this.makeMesh(this.getMaterial('parts', rect, { ambient: 0.4 }), 112, 97);
      mesh.visible = false;
      this.levelGroup.add(mesh);
      const glow = this.makeGlowMesh(relic.id === 1 ? 0xff5544 : 0x55aaff, 70);
      glow.visible = false;
      this.levelGroup.add(glow);
      this.relicViews.push({ relic, mesh, glow });
    });
  }

  makeGlowMesh(color, size, ring = false) {
    const mat = new THREE.MeshBasicMaterial({
      map: ring ? this.ringTex : this.glowTex,
      color,
      transparent: true,
      blending: THREE.AdditiveBlending,
      depthTest: false,
      depthWrite: false,
      opacity: 0.5
    });
    const mesh = new THREE.Mesh(this.quad, mat);
    mesh.scale.set(size, size, 1);
    return mesh;
  }

  placeGlow(mesh, cx, cy, size, order) {
    mesh.scale.set(size, size, 1);
    mesh.position.set(Math.round(cx - size / 2), Math.round(cy + size / 2), 0);
    mesh.renderOrder = order;
  }

  // Updates which tiles, items and markers are shown for the current level.
  syncCells(ds, lvl) {
    const W = ds.N + 2;
    for (const view of this.cellViews) {
      if (!view) continue;
      const cell = ds.cell(view.x, view.y, lvl);
      const show = !!cell.explored;
      view.tile.visible = show;
      if (view.stair) view.stair.visible = show;

      const padKey = cell.pad;
      if (padKey !== view.padKey) {
        view.padKey = padKey;
        if (view.pad) { this.levelGroup.remove(view.pad); view.pad = null; }
        if (padKey >= 0) {
          view.pad = this.makeMesh(this.getMaterial('parts', PAD_ART[padKey], { ambient: 0.4 }), PAD_ART[padKey][2], PAD_ART[padKey][3]);
          this.placeTopLeft(view.pad, ISO.originX(view.x, view.y), ISO.originY(view.x, view.y) + 20, this.orderKey(ds, view.x, view.y, 1));
          this.levelGroup.add(view.pad);
        }
      }
      if (view.pad) view.pad.visible = show;

      const itemKey = cell.item || null;
      if (itemKey !== view.itemKey) {
        view.itemKey = itemKey;
        if (view.item) { this.levelGroup.remove(view.item); view.item = null; }
        const art = itemKey && ITEM_ART[itemKey];
        if (art) {
          view.item = this.makeMesh(this.getMaterial('parts', art.rect, { ambient: 0.4 }), art.rect[2], art.rect[3]);
          view.item.userData.baseY = ISO.originY(view.x, view.y) + art.dy;
          this.placeTopLeft(view.item, ISO.originX(view.x, view.y), view.item.userData.baseY, this.orderKey(ds, view.x, view.y, 2));
          this.levelGroup.add(view.item);
        }
      }
      if (view.item) view.item.visible = show;
    }
  }

  // ---------- per frame ----------

  getView(entity) {
    let v = this.views.get(entity.id);
    if (!v) {
      const mat = this.makeMaterial('creatures', [0, 0, 64, 64], { ambient: 0.0 });
      v = {
        mesh: this.makeMesh(mat, 64, 64),
        shadow: this.makeShadowMesh(),
        shield: this.makeGlowMesh(0x66ddff, 80, true),
        relic: null
      };
      this.entityGroup.add(v.shadow, v.mesh, v.shield);
      this.views.set(entity.id, v);
    }
    return v;
  }

  makeShadowMesh() {
    const mat = new THREE.MeshBasicMaterial({ map: this.shadowTex, transparent: true, depthTest: false, depthWrite: false });
    const mesh = new THREE.Mesh(this.quad, mat);
    mesh.scale.set(46, 23, 1);
    return mesh;
  }

  clearEntities() {
    for (const [, v] of this.views) {
      this.entityGroup.remove(v.mesh, v.shadow, v.shield);
      if (v.relic) this.entityGroup.remove(v.relic);
    }
    this.views.clear();
  }

  hideAllViews() {
    this.views.forEach((v) => { v.mesh.visible = false; v.shadow.visible = false; v.shield.visible = false; if (v.relic) v.relic.visible = false; });
  }

  entityVisibility(ds, e, player) {
    if (!ds.cell(e.x, e.y, e.lvl).explored && !(e.fromX !== e.x && ds.cell(e.fromX, e.fromY, e.lvl).explored)) return 0;
    if (e === player) return 1;
    const p = this.gridToWorld(player.renderCell().x, player.renderCell().y);
    const rc = e.renderCell();
    const w = this.gridToWorld(rc.x, rc.y);
    const dx = w.x - p.x;
    const dy = (w.y - p.y) * 2;
    const dist = Math.sqrt(dx * dx + dy * dy);
    const r = this.lightRadius * 0.92;
    if (dist >= r) return 0;
    return Math.min(1, (r - dist) / (r * 0.35));
  }

  updateEntities(state) {
    const { ds, lvl, entities, player } = state;
    this.hideAllViews();

    for (const e of entities) {
      if (e.lvl !== lvl) continue;
      const dying = !e.alive;
      if (dying && e.deathTimer <= 0) continue;
      const vis = this.entityVisibility(ds, e, player);
      if (vis <= 0) continue;

      const v = this.getView(e);
      const rc = e.renderCell();
      const w = this.gridToWorld(rc.x, rc.y);
      const hover = e.image === 2 || e.image === 9 ? Math.sin(this.time * 3 + e.bob) * 3 : 0;
      const px = Math.round(w.x + ISO.spriteDX);
      const baseTop = Math.round(-w.y + ISO.spriteDY - hover);

      const cellOrder = this.orderKey(ds, e.moveT < 0.5 ? e.fromX : e.x, e.moveT < 0.5 ? e.fromY : e.y, 4);
      let alpha = vis;
      let squash = 1;
      if (dying) {
        const k = Math.max(0, e.deathTimer / 0.8);
        squash = Math.max(0.05, k);
        alpha *= Math.min(1, k * 1.5);
      }

      const mat = v.mesh.material;
      this.setRect(mat, e.spriteRect());
      mat.uniforms.uAlpha.value = alpha;
      mat.uniforms.uFlash.value = e.hitFlash > 0 ? Math.min(1, e.hitFlash / 0.12) * 0.7 : 0;
      mat.uniforms.uTint.value.set(1, 1, 1);
      v.mesh.scale.set(64, 64 * squash, 1);
      v.mesh.position.set(px, -(baseTop + 64 * (1 - squash)), 0);
      v.mesh.renderOrder = cellOrder;
      v.mesh.visible = true;

      v.shadow.visible = true;
      v.shadow.material.opacity = Math.min(1, alpha);
      v.shadow.position.set(Math.round(w.x - 23), Math.round(w.y + 11), 0);
      v.shadow.renderOrder = cellOrder - 1;

      if (e.protection > 0 && !dying) {
        v.shield.visible = true;
        const pulse = 74 + Math.sin(this.time * 9) * 4;
        v.shield.material.opacity = e.protection < 1.5 ? (Math.sin(this.time * 30) > 0 ? 0.8 : 0.2) : 0.75;
        this.placeGlow(v.shield, w.x, w.y + 30, pulse, cellOrder + 1);
      }

      if (v.relic && v.relicId !== e.carrying) {
        this.entityGroup.remove(v.relic);
        v.relic = null;
      }
      if (e.carrying && !dying) {
        if (!v.relic) {
          const rect = [e.carrying === 1 ? 0 : 678, 294, 112, 97];
          v.relic = this.makeMesh(this.makeMaterial('parts', rect, { ambient: 0.9 }), 56, 49);
          v.relicId = e.carrying;
          this.entityGroup.add(v.relic);
        }
        v.relic.visible = true;
        v.relic.material.uniforms.uAlpha.value = alpha;
        v.relic.position.set(Math.round(w.x - 28), Math.round(w.y + 78 + Math.sin(this.time * 4) * 2), 0);
        v.relic.renderOrder = cellOrder + 2;
      }
    }

    // forget views for entities that no longer exist
    if (this.views.size > entities.length + 8) {
      const live = new Set(entities.map(e => e.id));
      for (const [id, v] of this.views) {
        if (!live.has(id)) {
          this.entityGroup.remove(v.mesh, v.shadow, v.shield);
          if (v.relic) this.entityGroup.remove(v.relic);
          this.views.delete(id);
        }
      }
    }
  }

  updateProjectiles(state) {
    const { ds, lvl, projectiles } = state;
    while (this.projViews.length < projectiles.length) {
      const mat = this.makeMaterial('creatures', [0, 640, 64, 64], { ambient: 0.0 });
      const view = { mesh: this.makeMesh(mat, 64, 64), glow: this.makeGlowMesh(0xffaa55, 56) };
      this.fxGroup.add(view.mesh, view.glow);
      this.projViews.push(view);
    }
    this.projViews.forEach((v, i) => {
      const p = projectiles[i];
      if (!p || p.lvl !== lvl) { v.mesh.visible = false; v.glow.visible = false; return; }
      const rc = p.renderCell();
      const w = this.gridToWorld(rc.x, rc.y);
      const order = this.orderKey(ds, p.x, p.y, 5);
      this.setRect(v.mesh.material, p.spriteRect());
      v.mesh.material.uniforms.uAmbient.value = 0.55;
      v.mesh.position.set(Math.round(w.x + ISO.spriteDX), -Math.round(-w.y + ISO.spriteDY), 0);
      v.mesh.renderOrder = order;
      v.mesh.visible = true;
      const colors = { 1: 0xcfe8ff, 2: 0xff7755, 3: 0xff8a2a, 4: 0x55c8ff, 5: 0xaaaaaa, 6: 0x66ff55 };
      v.glow.material.color.setHex(colors[p.kind] || 0xffffff);
      v.glow.visible = p.kind !== 5;
      this.placeGlow(v.glow, w.x, w.y + 30, 54 + Math.sin(this.time * 25 + p.spin) * 6, order + 1);
      v.glow.material.opacity = 0.55;
    });
  }

  updateRelicsAndPits(state) {
    const { ds, lvl } = state;
    for (const rv of this.relicViews) {
      const r = rv.relic;
      const show = r.lvl === lvl && !r.carrier && ds.cell(r.x, r.y, lvl).explored;
      rv.mesh.visible = !!show;
      rv.glow.visible = !!show;
      if (show) {
        const bob = Math.sin(this.time * 2.5 + r.id) * 2;
        this.placeTopLeft(rv.mesh, ISO.originX(r.x, r.y), ISO.originY(r.x, r.y) + 20 + bob, this.orderKey(ds, r.x, r.y, 2));
        const w = this.gridToWorld(r.x, r.y);
        rv.glow.material.opacity = 0.35 + Math.sin(this.time * 3 + r.id) * 0.12;
        this.placeGlow(rv.glow, w.x, w.y + 12, 90, this.orderKey(ds, r.x, r.y, 2) - 1);
      }
    }
    const livePits = ds.pits.filter(p => p.active && p.lvl === lvl && ds.cell(p.x, p.y, lvl).explored);
    this.pitViews.forEach((mesh, i) => {
      const p = livePits[i];
      mesh.visible = !!p;
      if (!p) return;
      const f = p.frame;
      const art = f <= 6 || f >= 30 ? PIT_ART[0] : f <= 12 || f >= 24 ? PIT_ART[1] : PIT_ART[2];
      mesh.material = this.getMaterial('parts', art, { ambient: 0.4 });
      this.placeTopLeft(mesh, ISO.originX(p.x, p.y), ISO.originY(p.x, p.y) + 20, this.orderKey(ds, p.x, p.y, 1));
    });
  }

  // Items bob gently; cheap enough to do every frame.
  animateItems() {
    for (const v of this.cellViews) {
      if (v && v.item && v.item.visible) {
        v.item.position.y = -Math.round(v.item.userData.baseY + Math.sin(this.time * 2.2 + v.x * 1.7 + v.y) * 2);
      }
    }
  }

  // ---------- effects ----------

  addShake(amount) {
    this.shake = Math.min(14, Math.max(this.shake, amount));
  }

  spawnParticle(wx, wy, vx, vy, life, size, color, grow = 0) {
    let p = this.particles.find(q => q.life <= 0);
    if (!p) {
      if (this.particles.length >= 220) return;
      const mesh = this.makeGlowMesh(0xffffff, 8);
      this.fxGroup.add(mesh);
      p = { mesh, life: 0 };
      this.particles.push(p);
    }
    p.x = wx; p.y = wy; p.vx = vx; p.vy = vy;
    p.life = p.max = life; p.size = size; p.grow = grow;
    p.mesh.material.color.setHex(color);
    p.mesh.visible = true;
  }

  burst(wx, wy, color, count, speed, life, size) {
    for (let i = 0; i < count; i++) {
      const a = Math.random() * Math.PI * 2;
      const s = speed * (0.4 + Math.random() * 0.8);
      this.spawnParticle(wx, wy, Math.cos(a) * s, Math.sin(a) * s * 0.6, life * (0.6 + Math.random() * 0.6), size, color, -size * 0.6);
    }
  }

  floatText(wx, wy, text, color) {
    const key = `${text}|${color}`;
    this.textCache = this.textCache || new Map();
    let tex = this.textCache.get(key);
    if (!tex) {
      const c = document.createElement('canvas');
      const g = c.getContext('2d');
      g.font = 'bold 22px "Courier New", monospace';
      const w = Math.ceil(g.measureText(text).width) + 8;
      c.width = w;
      c.height = 30;
      g.font = 'bold 22px "Courier New", monospace';
      g.textBaseline = 'middle';
      g.lineWidth = 4;
      g.strokeStyle = 'rgba(0,0,0,0.9)';
      g.strokeText(text, 4, 16);
      g.fillStyle = color;
      g.fillText(text, 4, 16);
      tex = new THREE.CanvasTexture(c);
      tex.minFilter = THREE.LinearFilter;
      tex.generateMipmaps = false;
      this.textCache.set(key, tex);
    }
    const mat = new THREE.MeshBasicMaterial({ map: tex, transparent: true, depthTest: false, depthWrite: false });
    const mesh = new THREE.Mesh(this.quad, mat);
    const w = tex.image.width / 2;
    const h = tex.image.height / 2;
    mesh.scale.set(w, h, 1);
    mesh.renderOrder = 100000;
    this.fxGroup.add(mesh);
    this.floaters.push({ mesh, x: wx - w / 2, y: wy, life: 1.1, max: 1.1 });
  }

  updateEffects(dt) {
    for (const p of this.particles) {
      if (p.life <= 0) continue;
      p.life -= dt;
      if (p.life <= 0) { p.mesh.visible = false; continue; }
      p.x += p.vx * dt;
      p.y += p.vy * dt;
      p.vx *= 0.96;
      p.vy *= 0.96;
      const k = p.life / p.max;
      const size = Math.max(1, p.size + p.grow * (1 - k));
      p.mesh.material.opacity = Math.min(1, k * 1.4);
      p.mesh.scale.set(size, size, 1);
      p.mesh.position.set(Math.round(p.x - size / 2), Math.round(p.y + size / 2), 0);
      p.mesh.renderOrder = 90000;
    }
    for (let i = this.floaters.length - 1; i >= 0; i--) {
      const f = this.floaters[i];
      f.life -= dt;
      if (f.life <= 0) {
        this.fxGroup.remove(f.mesh);
        f.mesh.material.dispose();
        this.floaters.splice(i, 1);
        continue;
      }
      f.y += 26 * dt;
      f.mesh.material.opacity = Math.min(1, f.life / (f.max * 0.4));
      f.mesh.position.set(Math.round(f.x), Math.round(f.y), 0);
    }
  }

  // ---------- main ----------

  setCamera(target, dt, snap = false) {
    if (snap || this.camSnap) {
      this.cam.x = target.x;
      this.cam.y = target.y;
      this.camSnap = false;
    } else {
      const k = 1 - Math.exp(-dt * 9);
      this.cam.x += (target.x - this.cam.x) * k;
      this.cam.y += (target.y - this.cam.y) * k;
    }
  }

  frame(state, dt) {
    this.time += dt;
    this.shared.uTime.value = this.time;
    const { player } = state;

    const pc = player.renderCell();
    const pw = this.gridToWorld(pc.x, pc.y);
    this.setCamera({ x: pw.x, y: pw.y + 18 }, dt || 0.016);
    this.shared.uLight.value.set(pw.x, pw.y + 10);

    this.updateEntities(state);
    this.updateProjectiles(state);
    this.updateRelicsAndPits(state);
    this.animateItems();
    this.updateEffects(dt);

    this.shake *= Math.pow(0.001, dt);
    if (this.shake < 0.2) this.shake = 0;
    const sx = this.shake ? (Math.random() - 0.5) * this.shake : 0;
    const sy = this.shake ? (Math.random() - 0.5) * this.shake : 0;
    const snapStep = 1 / (this.scale * this.renderer.getPixelRatio());
    this.camera.position.x = Math.round(this.cam.x / snapStep) * snapStep + sx;
    this.camera.position.y = Math.round(this.cam.y / snapStep) * snapStep + sy;
    this.renderer.render(this.scene, this.camera);
  }

  render() {
    this.renderer.render(this.scene, this.camera);
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { MurkRenderer, ISO };
}
