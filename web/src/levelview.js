import * as THREE from 'three';
import { T } from './dungeon.js';
import { Rng } from './rng.js';

export const WALL_H = 0.9;

function tileKind(d, L, x, y) {
	if (x < 0 || y < 0 || x >= d.w || y >= d.h) return T.VOID;
	return d.levels[L].tiles[y * d.w + x];
}

class GeoBuilder {
	constructor(withCenter) {
		this.pos = [];
		this.nor = [];
		this.uv = [];
		this.col = [];
		this.ctr = withCenter ? [] : null;
		this.idx = [];
	}
	quad(v, n, uvs, cols, cx, cz) {
		const base = this.pos.length / 3;
		for (let i = 0; i < 4; i++) {
			this.pos.push(v[i][0], v[i][1], v[i][2]);
			this.nor.push(n[0], n[1], n[2]);
			this.uv.push(uvs[i][0], uvs[i][1]);
			const c = cols[i];
			this.col.push(c, c, c);
			if (this.ctr) this.ctr.push(cx, cz);
		}
		this.idx.push(base, base + 1, base + 2, base, base + 2, base + 3);
	}
	build() {
		const g = new THREE.BufferGeometry();
		g.setAttribute('position', new THREE.Float32BufferAttribute(this.pos, 3));
		g.setAttribute('normal', new THREE.Float32BufferAttribute(this.nor, 3));
		g.setAttribute('uv', new THREE.Float32BufferAttribute(this.uv, 2));
		g.setAttribute('color', new THREE.Float32BufferAttribute(this.col, 3));
		if (this.ctr) g.setAttribute('aCenter', new THREE.Float32BufferAttribute(this.ctr, 2));
		g.setIndex(this.idx);
		g.computeBoundingSphere();
		return g;
	}
}

/**
 * Build merged floor / wall-top / wall-side geometry for one dungeon level, plus
 * a list of wall torch positions.
 */
export function buildLevelGeometry(dungeon, L, seed = 1) {
	const rng = new Rng(seed * 7919 + L * 104729 + 17);
	const { w, h } = dungeon;
	const floor = new GeoBuilder(false);
	const tops = new GeoBuilder(true);
	const sides = new GeoBuilder(true);
	const torches = [];
	const isFloor = (x, y) => tileKind(dungeon, L, x, y) !== T.VOID;
	const faceTorch = [];

	for (let j = 0; j < h; j++) {
		for (let i = 0; i < w; i++) {
			if (isFloor(i, j)) {
				const x0 = i - 0.5;
				const x1 = i + 0.5;
				const z0 = j - 0.5;
				const z1 = j + 0.5;
				// soft corner occlusion from neighbouring wall cells
				const ao = (sx, sz) => {
					let n = 0;
					if (!isFloor(i + sx, j)) n++;
					if (!isFloor(i, j + sz)) n++;
					if (!isFloor(i + sx, j + sz)) n += 0.6;
					return 1 - 0.2 * n;
				};
				const c0 = ao(-1, -1);
				const c1 = ao(-1, 1);
				const c2 = ao(1, 1);
				const c3 = ao(1, -1);
				let uvs = [[0, 1], [0, 0], [1, 0], [1, 1]];
				const rot = rng.int(4);
				uvs = uvs.map((_, k) => uvs[(k + rot) % 4]);
				if (rng.chance(0.5)) uvs = [uvs[3], uvs[2], uvs[1], uvs[0]];
				floor.quad(
					[[x0, 0, z0], [x0, 0, z1], [x1, 0, z1], [x1, 0, z0]],
					[0, 1, 0], uvs, [c0, c1, c2, c3],
				);
			} else {
				let adj = false;
				for (let dy = -1; dy <= 1 && !adj; dy++) {
					for (let dx = -1; dx <= 1; dx++) {
						if ((dx || dy) && isFloor(i + dx, j + dy)) {
							adj = true;
							break;
						}
					}
				}
				if (!adj) continue;
				const x0 = i - 0.5;
				const x1 = i + 0.5;
				const z0 = j - 0.5;
				const z1 = j + 0.5;
				const H = WALL_H;
				tops.quad(
					[[x0, H, z0], [x0, H, z1], [x1, H, z1], [x1, H, z0]],
					[0, 1, 0], [[0, 1], [0, 0], [1, 0], [1, 1]], [0.8, 0.8, 0.8, 0.8], i, j,
				);
				// faces toward the camera: +z and +x
				for (const face of [[0, 1], [1, 0]]) {
					const [nx, nz] = face;
					const u = rng.chance(0.5) ? [1, 0] : [0, 1];
					const verts = nz
						? [[x0, 0, z1], [x1, 0, z1], [x1, H, z1], [x0, H, z1]]
						: [[x1, 0, z1], [x1, 0, z0], [x1, H, z0], [x1, H, z1]];
					sides.quad(
						verts, [nx, 0, nz], [[u[0], 0], [u[1], 0], [u[1], 1], [u[0], 1]], [0.55, 0.55, 1, 1], i, j,
					);
					if (isFloor(i + nx, j + nz)) faceTorch.push({ i, j, nx, nz });
				}
			}
		}
	}

	rng.shuffle(faceTorch);
	for (const f of faceTorch) {
		if (torches.some((t) => Math.hypot(t.x - f.i, t.z - f.j) < 3.2)) continue;
		if (!rng.chance(0.55)) continue;
		torches.push({
			x: f.i + f.nx * 0.5, z: f.j + f.nz * 0.5, nx: f.nx, nz: f.nz, ox: f.i + f.nx * 0.58, oz: f.j + f.nz * 0.58,
			phase: rng.float(0, 10),
		});
	}

	return {
		floor: floor.build(),
		tops: tops.pos.length ? tops.build() : null,
		sides: sides.pos.length ? sides.build() : null,
		torches,
	};
}

/** Inject fog-of-war sampling (and optional wall cut-away) into a built-in material. */
export function patchMaterial(material, shared, { cutaway = false } = {}) {
	material.onBeforeCompile = (shader) => {
		shader.uniforms.uFogTex = shared.uFogTex;
		shader.uniforms.uFogSize = shared.uFogSize;
		shader.uniforms.uFocus = shared.uFocus;
		shader.uniforms.uTime = shared.uTime;
		shader.vertexShader =
			(cutaway ? 'attribute vec2 aCenter;\n' : '') +
			'uniform vec3 uFocus;\nvarying vec2 vFogXZ;\n' +
			shader.vertexShader.replace(
				'#include <begin_vertex>',
				`#include <begin_vertex>
				${cutaway ? `
				vec2 dd = aCenter - uFocus.xz;
				float fwd = dot(dd, vec2(0.7071, 0.7071));
				float side = abs(dot(dd, vec2(0.7071, -0.7071)));
				float cut = smoothstep(-0.1, 0.9, fwd) * (1.0 - smoothstep(1.5, 2.6, side)) * (1.0 - smoothstep(3.6, 5.2, fwd));
				transformed.y *= mix(1.0, 0.16, cut);
				` : ''}
				vFogXZ = (modelMatrix * vec4(transformed, 1.0)).xz;`,
			);
		shader.fragmentShader =
			'uniform sampler2D uFogTex;\nuniform vec2 uFogSize;\nvarying vec2 vFogXZ;\n' +
			shader.fragmentShader.replace(
				'#include <opaque_fragment>',
				`float fogV = texture2D(uFogTex, (vFogXZ + 0.5) / uFogSize).r;
				float lum = dot(outgoingLight, vec3(0.299, 0.587, 0.114));
				outgoingLight = mix(vec3(lum) * vec3(0.8, 0.9, 1.1), outgoingLight, smoothstep(0.3, 0.95, fogV));
				outgoingLight *= fogV;
				#include <opaque_fragment>`,
			);
	};
	material.customProgramCacheKey = () => (cutaway ? 'fog-cut' : 'fog');
	return material;
}
