import * as THREE from 'three';
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';
import { ShaderPass } from 'three/addons/postprocessing/ShaderPass.js';
import { OutputPass } from 'three/addons/postprocessing/OutputPass.js';
import { T } from './dungeon.js';
import { THEME_STYLE, MISSILES, TUNING } from './config.js';
import { buildLevelGeometry, patchMaterial, WALL_H } from './levelview.js';
import { glowTexture, shadowBlob, flameTexture, padTexture, pitTexture } from './proc.js';
import { Particles } from './fx.js';

const PPU = 138.6; // pixels per world unit in the 2x sprite sheets (98px tile diamond / sqrt2, doubled)
const COS_E = Math.cos(Math.PI / 6);
const SHIFT = 0.35; // pull billboards toward the camera so they win depth ties against nearby walls
const R = Math.SQRT1_2;

const GradeShader = {
	uniforms: {
		tDiffuse: { value: null },
		uVig: { value: 0.62 },
		uHurt: { value: 0 },
		uTime: { value: 0 },
		uTint: { value: new THREE.Color(1, 1, 1) },
	},
	vertexShader: 'varying vec2 vUv; void main(){ vUv = uv; gl_Position = projectionMatrix * modelViewMatrix * vec4(position,1.0); }',
	fragmentShader: `
		uniform sampler2D tDiffuse; uniform float uVig; uniform float uHurt; uniform float uTime; uniform vec3 uTint;
		varying vec2 vUv;
		float hash(vec2 p){ return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453); }
		void main(){
			vec4 c = texture2D(tDiffuse, vUv);
			vec2 q = vUv - 0.5;
			float v = smoothstep(0.95, 0.25, length(q * vec2(1.0, 1.15)));
			c.rgb *= mix(1.0 - uVig, 1.0, v);
			c.rgb *= uTint;
			c.rgb = mix(c.rgb, vec3(c.r * 1.4 + 0.12, c.g * 0.4, c.b * 0.4), uHurt * (1.0 - v * 0.6));
			c.rgb += (hash(vUv * 900.0 + uTime) - 0.5) * 0.006;
			gl_FragColor = c;
		}`,
};

export class GameView {
	constructor(canvas, assets = null) {
		this.canvas = canvas;
		this.assets = assets;
		const renderer = new THREE.WebGLRenderer({ canvas, antialias: false, powerPreference: 'high-performance' });
		renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
		renderer.localClippingEnabled = true;
		renderer.setClearColor(0x000000, 1);
		this.renderer = renderer;

		this.scene = new THREE.Scene();
		this.camera = new THREE.OrthographicCamera(-1, 1, 1, -1, -200, 200);
		this.viewH = 9.5;
		this.targetViewH = 9.5;
		this.camTarget = new THREE.Vector3();
		this.camPos = new THREE.Vector3();
		this.shakeAmt = 0;
		this.hurt = 0;
		this.time = 0;
		this.world = null;
		this.levelViews = new Map();
		this.entityViews = new Map();
		this.missileViews = [];
		this.chestViews = new Map();
		this.lightLevel = 1;

		// shared uniforms for fog-of-war / cut-away patches
		this.shared = {
			uFogTex: { value: null },
			uFogSize: { value: new THREE.Vector2(1, 1) },
			uFocus: { value: new THREE.Vector3() },
			uTime: { value: 0 },
		};

		this.tex = {
			shadow: shadowBlob(),
			flame: flameTexture(),
			glow: glowTexture('#ffffff'),
			glowWarm: glowTexture('#ffb060'),
			pit: pitTexture(),
			pads: {},
		};
		this.clipPlane = new THREE.Plane(new THREE.Vector3(0, 1, 0), 0.01);

		this.ambient = new THREE.AmbientLight(0x303848, 1.7);
		this.hemi = new THREE.HemisphereLight(0x6070a0, 0x20150a, 0.7);
		this.scene.add(this.ambient, this.hemi);
		this.playerLight = new THREE.PointLight(0xffb070, 26, 12, 1.9);
		this.scene.add(this.playerLight);
		this.torchLights = [];
		for (let i = 0; i < 6; i++) {
			const l = new THREE.PointLight(0xff9040, 0, 8, 2);
			this.scene.add(l);
			this.torchLights.push(l);
		}
		this.fxLights = [];
		for (let i = 0; i < 4; i++) {
			const l = new THREE.PointLight(0xffffff, 0, 6, 2);
			this.scene.add(l);
			this.fxLights.push(l);
		}

		this.particles = new Particles(this.scene);
		this.fogData = null;
		this.fogCur = null;

		// post processing
		const size = new THREE.Vector2();
		renderer.getSize(size);
		const rt = new THREE.WebGLRenderTarget(4, 4, { type: THREE.HalfFloatType, samples: 4 });
		this.composer = new EffectComposer(renderer, rt);
		this.composer.addPass(new RenderPass(this.scene, this.camera));
		this.bloom = new UnrealBloomPass(new THREE.Vector2(256, 256), 0.5, 0.55, 0.82);
		this.composer.addPass(this.bloom);
		this.grade = new ShaderPass(GradeShader);
		this.composer.addPass(this.grade);
		this.composer.addPass(new OutputPass());

		this.floatSpawner = null;
		this.resize();
		window.addEventListener('resize', () => this.resize());
	}

	// ----------------------------------------------------------------- sizing
	resize() {
		const w = this.canvas.clientWidth || window.innerWidth;
		const h = this.canvas.clientHeight || window.innerHeight;
		this.renderer.setSize(w, h, false);
		this.composer.setSize(w, h);
		this.bloom.setSize(w, h);
		this.aspect = w / h;
		this.pxH = h * this.renderer.getPixelRatio();
		this.applyCamera();
	}

	applyCamera() {
		const v = this.viewH;
		const a = this.aspect || 1;
		const c = this.camera;
		c.left = -v * a / 2;
		c.right = v * a / 2;
		c.top = v / 2;
		c.bottom = -v / 2;
		c.updateProjectionMatrix();
		this.particles.setScale(this.pxH / v);
	}

	zoom(delta) {
		this.targetViewH = Math.min(18, Math.max(6.5, this.targetViewH * (1 + delta * 0.0012)));
	}

	shake(a) {
		this.shakeAmt = Math.min(1.2, Math.max(this.shakeAmt, a));
	}

	project(x, y, z) {
		const v = new THREE.Vector3(x, y, z).project(this.camera);
		const r = this.canvas.getBoundingClientRect();
		return { x: r.left + ((v.x + 1) / 2) * r.width, y: r.top + ((1 - v.y) / 2) * r.height };
	}

	/** Mouse pixel -> grid coordinates on the floor plane. */
	screenToGrid(cx, cy, height = 0.35) {
		const r = this.canvas.getBoundingClientRect();
		const nx = ((cx - r.left) / r.width) * 2 - 1;
		const ny = -((cy - r.top) / r.height) * 2 + 1;
		const ray = new THREE.Raycaster();
		ray.setFromCamera(new THREE.Vector2(nx, ny), this.camera);
		const plane = new THREE.Plane(new THREE.Vector3(0, 1, 0), -height);
		const out = new THREE.Vector3();
		if (!ray.ray.intersectPlane(plane, out)) return null;
		return { x: out.x, y: out.z };
	}

	// ----------------------------------------------------------------- sprites
	makeSprite(texture, cellW, cellH, anchorX, anchorY, opts = {}) {
		const geo = new THREE.PlaneGeometry(1, 1);
		const mat = new THREE.MeshBasicMaterial({
			map: texture, transparent: !!opts.blend, alphaTest: opts.blend ? 0 : 0.45, side: THREE.DoubleSide,
			depthWrite: !opts.blend, blending: opts.additive ? THREE.AdditiveBlending : THREE.NormalBlending,
			clippingPlanes: opts.clip ? [this.clipPlane] : null, color: 0xffffff,
		});
		if (opts.fog) patchMaterial(mat, this.shared);
		const mesh = new THREE.Mesh(geo, mat);
		mesh.rotation.y = Math.PI / 4;
		const W = (cellW / PPU) * (opts.scale || 1);
		const H = (cellH / (PPU * COS_E)) * (opts.scale || 1);
		mesh.scale.set(W, H, 1);
		const sp = {
			mesh, geo, mat, W, H, cellW, cellH, ax: anchorX, ay: anchorY, scale: opts.scale || 1, shift: opts.noShift ? 0 : SHIFT,
			texW: texture.image.width, texH: texture.image.height,
		};
		mesh.userData.sprite = sp;
		return sp;
	}

	setFrame(sp, px, py, pw, ph) {
		// pixel rect (top-left origin) in the source atlas -> uv
		const uv = sp.geo.attributes.uv;
		const e = 0.4;
		const u0 = (px + e) / sp.texW;
		const u1 = (px + pw - e) / sp.texW;
		const v1 = 1 - (py + e) / sp.texH;
		const v0 = 1 - (py + ph - e) / sp.texH;
		uv.setXY(0, u0, v1);
		uv.setXY(1, u1, v1);
		uv.setXY(2, u0, v0);
		uv.setXY(3, u1, v0);
		uv.needsUpdate = true;
	}

	placeSprite(sp, x, y, z, lift = 0) {
		const offX = ((sp.cellW / 2 - sp.ax) / PPU) * sp.scale;
		const offY = (((sp.ay - sp.cellH / 2) / (PPU * COS_E)) * sp.scale) + lift;
		sp.mesh.position.set(x + offX * R + sp.shift, y + offY + sp.shift * 0.8165, z - offX * R + sp.shift);
	}

	flatDecal(texture, size, color = 0xffffff, additive = false) {
		const geo = new THREE.PlaneGeometry(size, size);
		geo.rotateX(-Math.PI / 2);
		const mat = new THREE.MeshBasicMaterial({
			map: texture, transparent: true, depthWrite: false, color,
			blending: additive ? THREE.AdditiveBlending : THREE.NormalBlending,
			polygonOffset: true, polygonOffsetFactor: -2, polygonOffsetUnits: -2,
		});
		patchMaterial(mat, this.shared);
		const m = new THREE.Mesh(geo, mat);
		m.renderOrder = 2;
		return m;
	}

	// ------------------------------------------------------------------ world
	setWorld(world) {
		this.world = world;
		for (const lv of this.levelViews.values()) {
			this.scene.remove(lv.group);
			lv.dispose();
		}
		this.levelViews.clear();
		for (const ev of this.entityViews.values()) this.scene.remove(ev.group);
		this.entityViews.clear();
		for (const cv of this.chestViews.values()) this.scene.remove(cv.sp.mesh);
		this.chestViews.clear();
		for (const mv of this.missileViews) this.scene.remove(mv.sp.mesh);
		this.missileViews = [];
		this.particles.clear();
		this.currentLevel = -1;

		const { w, h } = world.dungeon;
		this.fogData = new Uint8Array(w * h);
		this.fogCur = new Float32Array(w * h);
		const tex = new THREE.DataTexture(this.fogData, w, h, THREE.RedFormat, THREE.UnsignedByteType);
		tex.magFilter = THREE.LinearFilter;
		tex.minFilter = THREE.LinearFilter;
		tex.needsUpdate = true;
		this.fogTex = tex;
		this.shared.uFogTex.value = tex;
		this.shared.uFogSize.value.set(w, h);

		world.on((type, data) => this.onEvent(type, data));
		const p = world.player;
		this.camTarget.set(p.x, 0, p.y);
		this.snapCamera = true;
		this.showLevel(p.level);
	}

	showLevel(L) {
		if (this.currentLevel === L) return;
		const old = this.levelViews.get(this.currentLevel);
		if (old) this.scene.remove(old.group);
		let lv = this.levelViews.get(L);
		if (!lv) {
			lv = this.buildLevel(L);
			this.levelViews.set(L, lv);
		}
		this.scene.add(lv.group);
		this.currentLevel = L;
		this.fogCur.fill(0);
		const style = THEME_STYLE[lv.theme] || THEME_STYLE.stone;
		this.ambient.color.set(style.ambient);
		this.playerLight.color.set(style.light);
		this.themeStyle = style;
		this.torchColor = new THREE.Color(style.light);
		for (const l of this.torchLights) l.color.copy(this.torchColor);
		this.renderer.setClearColor(0x000000, 1);
		for (const ev of this.entityViews.values()) ev.group.visible = false;
	}

	buildLevel(L) {
		const world = this.world;
		const d = world.dungeon;
		const theme = d.levels[L].theme;
		const style = THEME_STYLE[theme] || THEME_STYLE.stone;
		const th = this.assets.themes[theme];
		const geo = buildLevelGeometry(d, L, world.rng.int(1e6));
		const group = new THREE.Group();
		const disposables = [];

		const tint = new THREE.Color(style.tint);
		const floorMat = patchMaterial(new THREE.MeshStandardMaterial({
			map: th.floor, vertexColors: true, roughness: 0.92, metalness: 0, color: tint,
		}), this.shared);
		const floorMesh = new THREE.Mesh(geo.floor, floorMat);
		group.add(floorMesh);
		disposables.push(floorMat, geo.floor);

		if (geo.sides) {
			const wallMat = patchMaterial(new THREE.MeshStandardMaterial({
				map: th.wall, vertexColors: true, roughness: 0.95, metalness: 0, color: tint.clone().multiplyScalar(1.05),
			}), this.shared, { cutaway: true });
			group.add(new THREE.Mesh(geo.sides, wallMat));
			disposables.push(wallMat, geo.sides);
		}
		if (geo.tops) {
			const topMat = patchMaterial(new THREE.MeshStandardMaterial({
				map: th.floor, vertexColors: true, roughness: 1, metalness: 0, color: tint.clone().multiplyScalar(0.5),
			}), this.shared, { cutaway: true });
			group.add(new THREE.Mesh(geo.tops, topMat));
			disposables.push(topMat, geo.tops);
		}

		// torches
		const torches = [];
		for (const t of geo.torches) {
			const flame = this.makeSprite(this.tex.flame, 64, 128, 32, 100, { blend: true, additive: true, scale: 0.6, noShift: true, fog: true });
			flame.mesh.material.depthWrite = false;
			flame.mesh.position.set(t.ox + t.nx * 0.05, 0.78, t.oz + t.nz * 0.05);
			group.add(flame.mesh);
			torches.push({ ...t, flame, y: 0.8 });
		}

		// stairs
		const stairMeshes = [];
		const { items: itemsMeta } = this.assets.itemsMeta;
		for (const s of d.levels[L].stairs) {
			const key = s.kind === 'down' ? 'stairsDown' : 'stairsUp';
			const sp = this.makeItemSprite(key, true);
			sp.shift = 0;
			const m = itemsMeta[key];
			void m;
			this.placeSprite(sp, s.x, 0.02, s.y);
			group.add(sp.mesh);
			const col = s.kind === 'down' ? 0xff9a40 : 0x70c8ff;
			const glow = this.flatDecal(this.tex.glow, 2.3, col, true);
			glow.position.set(s.x, 0.012, s.y);
			group.add(glow);
			stairMeshes.push({ sp, glow, s });
		}

		// pads
		const pads = [];
		for (const p of world.pads) {
			if (p.level !== L) continue;
			if (!this.tex.pads[p.color]) this.tex.pads[p.color] = padTexture(p.color);
			const m = this.flatDecal(this.tex.pads[p.color], 1.7, 0xffffff, true);
			m.position.set(p.x, 0.014, p.y);
			group.add(m);
			pads.push({ m, p });
		}
		// pits
		const pits = [];
		for (const p of world.pits) {
			if (p.level !== L) continue;
			const m = this.flatDecal(this.tex.pit, 1.15);
			m.position.set(p.x, 0.016, p.y);
			m.scale.setScalar(0.001);
			group.add(m);
			pits.push({ m, p });
		}
		return {
			group, theme, torches, stairMeshes, pads, pits, itemViews: new Map(),
			dispose() {
				for (const d of disposables) d.dispose();
			},
		};
	}

	makeItemSprite(name, fog = true) {
		const meta = this.assets.itemsMeta;
		const it = meta.items[name];
		const sp = this.makeSprite(this.assets.items, meta.cellW, meta.cellH, 0, 0, { fog });
		const bb = it.bbox;
		sp.ax = (bb[0] + bb[2]) / 2;
		sp.ay = bb[3] - 10;
		this.setFrame(sp, it.index * meta.cellW, 0, meta.cellW, meta.cellH);
		return sp;
	}

	// ----------------------------------------------------------------- frames
	heroFrame(base, dir, moving, animT, staticFrame = false, colOffset = 0) {
		const meta = this.assets.heroesMeta;
		const blk = meta.blocks[base.block];
		const rowOff = dir === 1 || dir === 2 ? 0 : 1;
		const colBase = dir === 1 || dir === 3 ? 0 : 6;
		const frame = staticFrame ? colOffset : moving ? Math.floor(animT * 9) % 6 : 0;
		return { x: (colBase + frame) * 128, y: (blk.row + rowOff) * 128 };
	}

	/** Mirror of the wall cut-away term in the vertex shader (0 = full height, 1 = lowered). */
	cutAt(x, z) {
		const p = this.world.player;
		const dx = x - p.x;
		const dz = z - p.y;
		const fwd = (dx + dz) * Math.SQRT1_2;
		const side = Math.abs((dx - dz) * Math.SQRT1_2);
		const ss = (a, b, v) => {
			const t = Math.min(1, Math.max(0, (v - a) / (b - a)));
			return t * t * (3 - 2 * t);
		};
		return ss(-0.1, 0.9, fwd) * (1 - ss(1.5, 2.6, side)) * (1 - ss(3.6, 5.2, fwd));
	}

	brightnessAt(x, z) {
		const p = this.world.player;
		let b = 0.62;
		const d = Math.hypot(x - p.x, z - p.y);
		b += 0.55 * Math.pow(Math.max(0, 1 - d / 9), 1.4);
		const lv = this.levelViews.get(this.currentLevel);
		if (lv) {
			for (const t of lv.torches) {
				const td = Math.hypot(x - t.x, z - t.z);
				if (td < 6) b += 0.5 * Math.pow(1 - td / 6, 2) * (0.85 + 0.15 * Math.sin(this.time * 9 + t.phase));
			}
		}
		return Math.min(1.15, b);
	}

	makeEntityView(e) {
		const group = new THREE.Group();
		const meta = this.assets.heroesMeta;
		const blk = meta.blocks[e.base.block];
		const foot = blk.foot * 128;
		const sp = this.makeSprite(this.assets.heroes, 128, 128, 64, foot, { clip: true });
		const f = this.heroFrame(e.base, e.dir, false, 0, !!e.base.staticFrame, e.base.col || 0);
		this.setFrame(sp, f.x, f.y, 128, 128);
		group.add(sp.mesh);

		const shadow = new THREE.Mesh(new THREE.PlaneGeometry(0.9, 0.9).rotateX(-Math.PI / 2),
			new THREE.MeshBasicMaterial({ map: this.tex.shadow, transparent: true, depthWrite: false, polygonOffset: true, polygonOffsetFactor: -1, polygonOffsetUnits: -1 }));
		shadow.renderOrder = 1;
		group.add(shadow);

		// ring under the feet shows allegiance
		const ringColor = e.isPlayer ? 0x6dff9a : e.kind === 'monster' ? 0xff4a3a : e.team === 1 ? 0x6ab4ff : 0xff5a5a;
		const ring = this.flatDecal(this.tex.glow, e.isPlayer ? 1.4 : 1.1, ringColor, true);
		ring.material.opacity = e.isPlayer ? 0.55 : 0.35;
		group.add(ring);

		// protection bubble
		const bubble = this.makeItemSprite('bubble', false);
		bubble.mat.transparent = true;
		bubble.mat.alphaTest = 0;
		bubble.mat.blending = THREE.AdditiveBlending;
		bubble.mat.depthWrite = false;
		bubble.mat.opacity = 0.0;
		bubble.mesh.visible = false;
		group.add(bubble.mesh);

		// health bar
		let bar = null;
		if (!e.isPlayer) {
			const bg = new THREE.Mesh(new THREE.PlaneGeometry(0.62, 0.075), new THREE.MeshBasicMaterial({ color: 0x120808, depthTest: false, transparent: true, opacity: 0.8 }));
			const fg = new THREE.Mesh(new THREE.PlaneGeometry(0.58, 0.045), new THREE.MeshBasicMaterial({ color: 0xff3a2a, depthTest: false }));
			bg.rotation.y = fg.rotation.y = Math.PI / 4;
			bg.renderOrder = 20;
			fg.renderOrder = 21;
			group.add(bg, fg);
			bar = { bg, fg };
		}
		this.scene.add(group);
		return { group, sp, shadow, ring, bubble, bar, e, top: blk.top * 128, foot };
	}

	updateEntityViews(dt) {
		const world = this.world;
		const L = world.player.level;
		for (const ev of this.entityViews.values()) {
			if (ev.e.level !== L) ev.group.visible = false;
		}
		for (const e of world.entities) {
			if (e.level !== L) continue;
			let ev = this.entityViews.get(e.id);
			if (!ev) {
				ev = this.makeEntityView(e);
				this.entityViews.set(e.id, ev);
			}
			const seen = e.isPlayer || world.isVisible(e.level, e.x, e.y) || Math.hypot(e.x - world.player.x, e.y - world.player.y) < 1.6;
			const sinkT = e.alive ? 0 : Math.min(1, e.deathT / 1.1);
			const hide = !seen || (!e.alive && e.deathT > 1.25);
			ev.group.visible = !hide;
			if (hide) continue;

			const f = this.heroFrame(e.base, e.dir, e.moving, e.animT, !!e.base.staticFrame, e.base.col || 0);
			this.setFrame(ev.sp, f.x, f.y, 128, 128);
			const hover = e.base.hover ? 0.3 + Math.sin(e.bob || 0) * 0.07 : 0;
			const sink = sinkT * (ev.sp.H * 0.8);
			this.placeSprite(ev.sp, e.x, hover - sink, e.y);
			ev.shadow.position.set(e.x, 0.011, e.y);
			const ss = e.base.hover ? 0.75 - hover * 0.4 : 1;
			ev.shadow.scale.setScalar(ss * (1 - sinkT * 0.5));
			ev.ring.position.set(e.x, 0.013, e.y);
			ev.ring.visible = e.alive;

			const b = this.brightnessAt(e.x, e.y);
			let r = b;
			let g = b;
			let bl = b;
			if (e.flashT > 0) {
				const k = e.flashT / 0.18;
				r += k * 1.6;
				g += k * 0.5;
				bl += k * 0.5;
			}
			if (e.slowT > 0) {
				r *= 0.7;
				g *= 0.95;
				bl *= 1.4;
			}
			if (e.hitT > 0 && e.kind === 'hero' && e.alive && Math.floor(this.time * 20) % 2 === 0) {
				r *= 0.5;
				g *= 0.5;
				bl *= 0.5;
			}
			ev.sp.mat.color.setRGB(r, g, bl);
			ev.sp.mat.opacity = 1;
			ev.sp.mat.alphaTest = 0.45;

			// protection bubble
			if (e.protT > 0) {
				ev.bubble.mesh.visible = true;
				ev.bubble.mat.opacity = 0.55 + 0.25 * Math.sin(this.time * 12);
				this.placeSprite(ev.bubble, e.x, 0, e.y);
				const bp = ev.bubble.mesh.position;
				bp.x += 0.05;
				bp.y += 0.2;
				ev.bubble.mat.color.setRGB(1, 1, 1);
				if (Math.random() < dt * 30) this.particles.emit({ x: e.x + (Math.random() - 0.5) * 0.6, y: 0.2, z: e.y + (Math.random() - 0.5) * 0.6, vy: 1.2, life: 0.5, size: 0.14, color: [1, 0.9, 0.4], alpha: 0.9 });
			} else ev.bubble.mesh.visible = false;

			if (ev.bar) {
				const show = e.alive && (e.hp < e.maxHp) && !e.asleep;
				ev.bar.bg.visible = ev.bar.fg.visible = show;
				if (show) {
					const frac = Math.max(0, e.hp / e.maxHp);
					const top = ((ev.foot - ev.top) / (PPU * COS_E)) * ev.sp.scale * 1.0 + hover + 0.12;
					const px = e.x + SHIFT + 0.1;
					const pz = e.y + SHIFT + 0.1;
					const py = top + SHIFT * 0.8165;
					ev.bar.bg.position.set(px, py, pz);
					ev.bar.fg.position.set(px, py, pz);
					ev.bar.fg.scale.x = Math.max(0.001, frac);
					ev.bar.fg.position.x -= (1 - frac) * 0.29 * R;
					ev.bar.fg.position.z += (1 - frac) * 0.29 * R;
					ev.bar.fg.material.color.setHSL(frac * 0.3, 0.9, 0.5);
				}
			}
		}
	}

	// ---------------------------------------------------------------- missiles
	updateMissileViews(dt) {
		const world = this.world;
		const L = world.player.level;
		const list = world.missiles.filter((m) => m.level === L);
		while (this.missileViews.length < list.length) {
			const sp = this.makeSprite(this.assets.heroes, 128, 128, 64, 64, { scale: 0.85 });
			sp.mat.alphaTest = 0.3;
			this.scene.add(sp.mesh);
			this.missileViews.push({ sp });
		}
		const lightsUsed = [];
		for (let i = 0; i < this.missileViews.length; i++) {
			const mv = this.missileViews[i];
			if (i >= list.length) {
				mv.sp.mesh.visible = false;
				continue;
			}
			const m = list[i];
			const meta = this.assets.heroesMeta;
			const row = meta.blocks.missiles.row;
			const dirRow = Math.abs(m.vx) >= Math.abs(m.vy) ? (m.vx < 0 ? 1 : 4) : m.vy < 0 ? 2 : 3;
			const rowOff = dirRow === 1 || dirRow === 2 ? 0 : 1;
			const colBase = dirRow === 1 || dirRow === 3 ? 0 : 6;
			this.setFrame(mv.sp, (colBase + m.image - 1) * 128, (row + rowOff) * 128, 128, 128);
			this.placeSprite(mv.sp, m.x, 0.55, m.y);
			mv.sp.mesh.visible = world.isVisible(m.level, m.x, m.y);
			const info = MISSILES[m.image];
			const bright = info.glow ? 1.5 : this.brightnessAt(m.x, m.y);
			mv.sp.mat.color.setRGB(bright, bright, bright);
			if (info.glow && mv.sp.mesh.visible) {
				const c = new THREE.Color(info.color);
				if (Math.random() < 0.9) {
					this.particles.emit({
						x: m.x + (Math.random() - 0.5) * 0.12, y: 0.55 + (Math.random() - 0.5) * 0.1, z: m.y + (Math.random() - 0.5) * 0.12,
						vx: -m.vx * 0.05, vz: -m.vy * 0.05, vy: 0.3, life: 0.35, size: 0.2, grow: -0.3, color: [c.r, c.g, c.b], alpha: 0.9,
					});
				}
				if (lightsUsed.length < this.fxLights.length) lightsUsed.push({ x: m.x, z: m.y, color: info.glow });
			}
		}
		this.fxLights.forEach((l, i) => {
			const u = lightsUsed[i];
			if (u) {
				l.intensity = 14;
				l.color.set(u.color);
				l.position.set(u.x, 0.7, u.z);
			} else l.intensity = this.flashLight && i === 0 ? this.flashLight.i : 0;
		});
		if (this.flashLight) {
			const fl = this.flashLight;
			fl.i *= Math.pow(0.002, dt);
			if (fl.i < 0.5) this.flashLight = null;
			else if (!lightsUsed[3]) {
				const l = this.fxLights[3];
				l.intensity = fl.i;
				l.color.set(fl.color);
				l.position.set(fl.x, 0.8, fl.z);
			}
		}
	}

	// -------------------------------------------------------- items & objects
	updateItemViews(dt) {
		const world = this.world;
		const L = world.player.level;
		const lv = this.levelViews.get(L);
		if (!lv) return;
		const items = world.items[L];
		const w = world.dungeon.w;
		const names = { elixir: 'potion', armour: 'armour', scroll: 'scroll', ring: 'ring' };
		for (const [key, it] of items) {
			let v = lv.itemViews.get(key);
			if (!v) {
				const sp = this.makeItemSprite(names[it.type], true);
				sp.mesh.scale.multiplyScalar(0.62);
				sp.scale = 0.62;
				lv.group.add(sp.mesh);
				const glow = this.flatDecal(this.tex.glow, 0.95, it.type === 'ring' ? 0xffe070 : it.type === 'elixir' ? 0xff6080 : 0x80b0ff, true);
				glow.material.opacity = 0.7;
				lv.group.add(glow);
				v = { sp, glow, it, x: key % w, y: Math.floor(key / w) };
				lv.itemViews.set(key, v);
			}
			const bob = Math.sin(this.time * 2.4 + (it.bob || 0)) * 0.05;
			this.placeSprite(v.sp, v.x, 0.12 + bob, v.y);
			v.glow.position.set(v.x, 0.013, v.y);
			v.glow.material.opacity = 0.5 + 0.2 * Math.sin(this.time * 3 + (it.bob || 0));
			const b = this.brightnessAt(v.x, v.y);
			v.sp.mat.color.setRGB(b * 1.1, b * 1.1, b * 1.1);
		}
		for (const [key, v] of lv.itemViews) {
			if (!items.has(key)) {
				lv.group.remove(v.sp.mesh, v.glow);
				v.sp.geo.dispose();
				lv.itemViews.delete(key);
			}
		}
		void dt;
	}

	updateChestViews(dt) {
		const world = this.world;
		const L = world.player.level;
		for (const c of world.chests) {
			let v = this.chestViews.get(c.id);
			if (!v) {
				const sp = this.makeItemSprite(c.color === 'red' ? 'chestRed' : 'chestBlue', true);
				this.scene.add(sp.mesh);
				const glow = this.flatDecal(this.tex.glow, 1.7, c.color === 'red' ? 0xff4040 : 0x4070ff, true);
				this.scene.add(glow);
				v = { sp, glow };
				this.chestViews.set(c.id, v);
			}
			const show = c.level === L && (world.explored[L][world.idx(Math.round(c.x), Math.round(c.y))] || c.carrier != null);
			v.sp.mesh.visible = !!show;
			v.glow.visible = !!show;
			if (!show) continue;
			let lift = Math.sin(c.bob * 2.2) * 0.04;
			let sc = 0.85;
			if (c.carrier != null) {
				lift += 1.15;
				sc = 0.6;
			}
			v.sp.mesh.scale.set(v.sp.W * (sc / v.sp.scale), v.sp.H * (sc / v.sp.scale), 1);
			const savedScale = v.sp.scale;
			v.sp.scale = sc;
			this.placeSprite(v.sp, c.x, 0.04 + lift, c.y);
			v.sp.scale = savedScale;
			v.glow.position.set(c.x, 0.013, c.y);
			v.glow.material.opacity = 0.55 + 0.25 * Math.sin(c.bob * 3);
			const b = Math.max(0.9, this.brightnessAt(c.x, c.y)) * 1.2;
			v.sp.mat.color.setRGB(b, b, b);
			if (Math.random() < dt * 14) {
				const col = c.color === 'red' ? [1, 0.45, 0.3] : [0.5, 0.7, 1];
				this.particles.emit({
					x: c.x + (Math.random() - 0.5) * 0.7, y: 0.1 + Math.random() * 0.4 + (c.carrier != null ? lift : 0),
					z: c.y + (Math.random() - 0.5) * 0.7, vy: 0.5 + Math.random() * 0.5, life: 1.2, size: 0.12, color: col, alpha: 0.9, fadeIn: 0.3,
				});
			}
		}
	}

	updateLevelObjects(dt) {
		const lv = this.levelViews.get(this.currentLevel);
		if (!lv) return;
		const world = this.world;
		const ex = world.explored[this.currentLevel];
		const w = world.dungeon.w;
		for (const t of lv.torches) {
			const flick = 0.85 + 0.15 * Math.sin(this.time * 11 + t.phase) + 0.08 * Math.sin(this.time * 27 + t.phase * 2);
			t.cut = this.cutAt(t.x, t.z);
			t.flame.mesh.scale.set(t.flame.W * (0.9 + 0.12 * flick), t.flame.H * (0.85 + 0.25 * flick), 1);
			t.flame.mesh.material.opacity = 1;
			const seen = ex[Math.round(t.z - t.nz * 0.5) * w + Math.round(t.x - t.nx * 0.5)];
			t.flame.mesh.visible = !!seen && t.cut < 0.4;
			t.flick = flick;
			if (seen && t.cut < 0.4 && Math.random() < dt * 3) {
				this.particles.emit({ x: t.ox, y: 0.9, z: t.oz, vx: (Math.random() - 0.5) * 0.3, vz: (Math.random() - 0.5) * 0.3, vy: 0.9, life: 0.9, size: 0.08, color: [1, 0.6, 0.2], alpha: 0.9 });
			}
		}
		for (const s of lv.stairMeshes) {
			const seen = ex[s.s.y * w + s.s.x];
			s.sp.mesh.visible = !!seen;
			s.glow.visible = !!seen;
			s.glow.material.opacity = 0.65 + 0.25 * Math.sin(this.time * 2.5);
			if (seen) {
				const b = Math.max(0.85, this.brightnessAt(s.s.x, s.s.y));
				s.sp.mat.color.setRGB(b, b, b);
			}
		}
		for (const p of lv.pads) {
			p.m.rotation.y += dt * 0.3;
			p.m.visible = !!ex[p.p.y * w + p.p.x];
			p.m.material.opacity = 0.8 + 0.2 * Math.sin(this.time * 2);
		}
		for (const p of lv.pits) {
			const o = p.p.open || 0;
			p.m.scale.setScalar(Math.max(0.001, o));
			p.m.visible = !!ex[p.p.y * w + p.p.x] && o > 0.01;
			// telegraph: dusty cracks before opening
			if (p.m.visible && Math.random() < dt * 8 && o < 1) {
				this.particles.emit({ x: p.p.x + (Math.random() - 0.5), y: 0.05, z: p.p.y + (Math.random() - 0.5), vy: 0.5, life: 0.6, size: 0.12, color: [0.5, 0.4, 0.3], add: false, alpha: 0.7 });
			}
		}
		void dt;
	}

	updateFog(dt) {
		const world = this.world;
		const L = world.player.level;
		const { w, h } = world.dungeon;
		const ex = world.explored[L];
		const vis = world.visible;
		const tiles = world.dungeon.levels[L].tiles;
		const k = 1 - Math.exp(-dt * 9);
		const target = (x, y) => {
			const i = y * w + x;
			if (vis[i] >= 0) return 1;
			if (ex[i]) return 0.36;
			return 0;
		};
		for (let y = 0; y < h; y++) {
			for (let x = 0; x < w; x++) {
				const i = y * w + x;
				let t;
				if (tiles[i] !== T.VOID) t = target(x, y);
				else {
					t = 0;
					for (let dy = -1; dy <= 1; dy++) {
						for (let dx = -1; dx <= 1; dx++) {
							const nx = x + dx;
							const ny = y + dy;
							if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
							if (tiles[ny * w + nx] !== T.VOID) t = Math.max(t, target(nx, ny));
						}
					}
				}
				this.fogCur[i] += (t - this.fogCur[i]) * k;
				this.fogData[i] = Math.round(Math.min(1, this.fogCur[i]) * 255);
			}
		}
		this.fogTex.needsUpdate = true;
	}

	// ------------------------------------------------------------------ events
	onEvent(type, d) {
		const P = this.particles;
		const world = this.world;
		if (!world || (d.entity && d.entity.level !== world.player.level && type !== 'stairs')) return;
		switch (type) {
			case 'fire': {
				const m = d.missile;
				const info = MISSILES[m.image];
				if (info.glow) {
					const c = new THREE.Color(info.color);
					P.burst(6, { x: m.x, y: 0.55, z: m.y, vy: 0.3, life: 0.35, size: 0.2, color: [c.r, c.g, c.b], alpha: 0.9 }, 1, 1.2);
				}
				break;
			}
			case 'impact': {
				const m = d.missile;
				if (m.level !== world.player.level || d.expired) return;
				const info = d.info;
				const c = new THREE.Color(info.color);
				P.burst(info.aoe ? 26 : 12, { x: m.x, y: 0.5, z: m.y, vy: 2, life: 0.55, size: info.aoe ? 0.32 : 0.16, color: [c.r, c.g, c.b], alpha: 1, gravity: 4, drag: 2 }, 1, info.aoe ? 3.4 : 2.2);
				if (info.aoe) {
					P.burst(10, { x: m.x, y: 0.4, z: m.y, vy: 0.6, life: 0.9, size: 0.45, grow: 0.8, color: [0.25, 0.22, 0.2], alpha: 0.5, add: false, drag: 1.5 }, 1, 1.4);
					this.flashLight = { x: m.x, z: m.y, i: 30, color: info.glow };
					this.shake(0.12);
				} else if (!d.target) {
					P.burst(6, { x: m.x, y: 0.4, z: m.y, vy: 1, life: 0.4, size: 0.08, color: [1, 0.9, 0.7], alpha: 1, gravity: 5 }, 1, 1.5);
				}
				break;
			}
			case 'hit': {
				const e = d.entity;
				const blood = e.kind === 'monster' && (e.type === 'skeleton') ? [0.9, 0.9, 0.8] : e.kind === 'monster' ? [0.4, 1, 0.5] : [1, 0.15, 0.1];
				P.burst(10, { x: e.x, y: 0.55, z: e.y, vy: 1.6, life: 0.6, size: 0.14, color: blood, alpha: 1, gravity: 5, add: false, drag: 1 }, 1, 2.4);
				if (this.floatSpawner) this.floatSpawner(e.x, 0.9, e.y, d.hurt > 0 ? `-${d.hurt}` : `${d.amount}`, e.isPlayer ? 'dmg-player' : d.hurt > 0 ? 'dmg' : 'armour');
				if (e.isPlayer) {
					this.hurt = Math.min(1, this.hurt + 0.55);
					this.shake(0.28);
				}
				break;
			}
			case 'death': {
				const e = d.entity;
				const c = e.kind === 'monster' ? [e.base.glow ? 1 : 0.8, 0.5, 0.3] : [1, 0.3, 0.2];
				P.burst(36, { x: e.x, y: 0.6, z: e.y, vy: 2.4, life: 1.0, size: 0.24, color: c, alpha: 1, gravity: 3, drag: 1.2 }, 1, 3);
				P.burst(14, { x: e.x, y: 0.3, z: e.y, vy: 0.8, life: 1.4, size: 0.5, grow: 0.7, color: [0.3, 0.27, 0.25], alpha: 0.55, add: false, drag: 1.6 }, 1, 1.2);
				if (e.isPlayer) this.shake(0.7);
				break;
			}
			case 'pickup': {
				const col = d.item.type === 'ring' ? [1, 0.9, 0.3] : d.item.type === 'elixir' ? [1, 0.4, 0.6] : [0.5, 0.7, 1];
				P.burst(18, { x: d.x, y: 0.3, z: d.y, vy: 2.5, life: 0.8, size: 0.18, color: col, alpha: 1, gravity: -1 }, 0.7, 1.6);
				if (this.floatSpawner) this.floatSpawner(d.x, 0.9, d.y, d.text, 'pickup');
				break;
			}
			case 'ring': {
				const e = d.entity;
				P.burst(40, { x: e.x, y: 0.5, z: e.y, vy: 0.5, life: 0.9, size: 0.2, color: [1, 0.9, 0.4], alpha: 1 }, 1, 4);
				this.flashLight = { x: e.x, z: e.y, i: 26, color: 0xffe080 };
				break;
			}
			case 'levelup': {
				const e = d.entity;
				for (let i = 0; i < 4; i++) P.burst(24, { x: e.x, y: 0.2 + i * 0.3, z: e.y, vy: 3, life: 1.2, size: 0.22, color: [1, 0.85, 0.3], alpha: 1, gravity: -0.5 }, 0.5, 2.4);
				this.flashLight = { x: e.x, z: e.y, i: 30, color: 0xffd060 };
				break;
			}
			case 'stairs': {
				const e = d.entity;
				if (e.isPlayer) {
					this.snapCamera = true;
					this.showLevel(d.level);
					this.flashTint = 1;
				}
				break;
			}
			case 'respawn': {
				const e = d.entity;
				if (e.isPlayer) this.snapCamera = true;
				P.burst(30, { x: e.x, y: 0.2, z: e.y, vy: 3, life: 1.0, size: 0.2, color: [0.6, 1, 0.7], alpha: 1, gravity: -1 }, 0.6, 1.8);
				break;
			}
			case 'fall': {
				if (d.entity.isPlayer) {
					this.showLevel(d.level);
					this.snapCamera = true;
					this.shake(0.9);
				}
				break;
			}
			case 'ringKill':
				this.shake(0.15);
				break;
			case 'alarm':
				this.shake(0.5);
				this.hurt = Math.max(this.hurt, 0.25);
				break;
			case 'chestTaken':
			case 'chestReturned':
			case 'chestDropped': {
				const c = d.chest;
				P.burst(26, { x: c.x, y: 0.4, z: c.y, vy: 2.2, life: 1.0, size: 0.2, color: c.color === 'red' ? [1, 0.5, 0.3] : [0.5, 0.7, 1], alpha: 1, gravity: -0.4 }, 0.6, 2);
				break;
			}
			default:
		}
	}

	// ------------------------------------------------------------------ update
	update(dt) {
		const world = this.world;
		if (!world) return;
		this.time += dt;
		const p = world.player;
		if (this.currentLevel !== p.level) this.showLevel(p.level);

		// smooth zoom, camera follow, shake
		this.viewH += (this.targetViewH - this.viewH) * (1 - Math.exp(-dt * 8));
		this.applyCamera();
		const look = p.aimLook || 0;
		void look;
		const tx = p.x;
		const tz = p.y;
		if (this.snapCamera) {
			this.camTarget.set(tx, 0, tz);
			this.snapCamera = false;
		} else {
			const k = 1 - Math.exp(-dt * 6.5);
			this.camTarget.x += (tx - this.camTarget.x) * k;
			this.camTarget.z += (tz - this.camTarget.z) * k;
		}
		this.shakeAmt *= Math.pow(0.02, dt);
		const sx = (Math.random() - 0.5) * this.shakeAmt * 0.5;
		const sz = (Math.random() - 0.5) * this.shakeAmt * 0.5;
		const cp = this.camTarget;
		this.camera.position.set(cp.x + sx + 40, 40 * 0.8165 + 0.3, cp.z + sz + 40);
		this.camera.lookAt(cp.x + sx, 0.3, cp.z + sz);

		this.shared.uFocus.value.set(p.x, 0, p.y);
		this.shared.uTime.value = this.time;

		// lighting
		const flick = 0.93 + 0.07 * Math.sin(this.time * 13) + 0.04 * Math.sin(this.time * 31);
		this.playerLight.position.set(p.x, 1.5, p.y);
		this.playerLight.intensity = 26 * flick;
		const lv = this.levelViews.get(this.currentLevel);
		if (lv) {
			const near = lv.torches
				.filter((t) => world.explored[this.currentLevel][Math.round(t.z - t.nz * 0.5) * world.dungeon.w + Math.round(t.x - t.nx * 0.5)])
				.map((t) => ({ t, d: Math.hypot(t.x - p.x, t.z - p.y) }))
				.sort((a, b) => a.d - b.d)
				.slice(0, this.torchLights.length);
			this.torchLights.forEach((l, i) => {
				const n = near[i];
				if (n) {
					l.position.set(n.t.x - n.t.nx * 0.1 + n.t.nx * 0.5, 0.85, n.t.z + n.t.nz * 0.5);
					l.intensity = 22 * (n.t.flick || 1) * (1 - (n.t.cut || 0));
				} else l.intensity = 0;
			});
		}

		this.updateFog(dt);
		this.updateLevelObjects(dt);
		this.updateItemViews(dt);
		this.updateChestViews(dt);
		this.updateEntityViews(dt);
		this.updateMissileViews(dt);

		// ambient dust motes drifting around the hero
		if (Math.random() < dt * 5) {
			this.particles.emit({
				x: p.x + (Math.random() - 0.5) * 12, y: 0.2 + Math.random() * 1.4, z: p.y + (Math.random() - 0.5) * 12,
				vx: (Math.random() - 0.5) * 0.15, vz: (Math.random() - 0.5) * 0.15, vy: 0.04, life: 4, size: 0.06, color: [1, 0.85, 0.6], alpha: 0.35, fadeIn: 0.4,
			});
		}
		this.particles.update(dt);

		this.hurt = Math.max(0, this.hurt - dt * 1.4);
		this.grade.uniforms.uHurt.value = this.hurt * 0.55 + (p.alive ? 0 : 0.35);
		this.grade.uniforms.uTime.value = this.time % 10;
		this.grade.uniforms.uTint.value.set(1, 1, 1);
		if (this.flashTint) {
			this.flashTint = Math.max(0, this.flashTint - dt * 3);
			const f = this.flashTint;
			this.grade.uniforms.uTint.value.setRGB(1 + f * 0.6, 1 + f * 0.6, 1 + f * 0.8);
		}
		void TUNING;
		void WALL_H;
	}

	render() {
		this.composer.render();
	}
}
