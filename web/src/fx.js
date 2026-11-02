import * as THREE from 'three';
import { softDot } from './proc.js';

const MAX = 2600;

/**
 * Lightweight CPU particle system rendered as two THREE.Points batches
 * (additive for magic/fire, normal for dust/debris).
 */
export class Particles {
	constructor(scene) {
		this.scene = scene;
		this.pxScale = 1;
		const tex = softDot();
		this.batches = [this.makeBatch(THREE.AdditiveBlending, tex), this.makeBatch(THREE.NormalBlending, tex)];
		this.rnd = Math.random;
	}

	makeBatch(blending, tex) {
		const geo = new THREE.BufferGeometry();
		const pos = new Float32Array(MAX * 3);
		const col = new Float32Array(MAX * 4);
		const size = new Float32Array(MAX);
		geo.setAttribute('position', new THREE.BufferAttribute(pos, 3).setUsage(THREE.DynamicDrawUsage));
		geo.setAttribute('aColor', new THREE.BufferAttribute(col, 4).setUsage(THREE.DynamicDrawUsage));
		geo.setAttribute('aSize', new THREE.BufferAttribute(size, 1).setUsage(THREE.DynamicDrawUsage));
		geo.setDrawRange(0, 0);
		const mat = new THREE.ShaderMaterial({
			uniforms: { uTex: { value: tex }, uScale: { value: 1 } },
			vertexShader: `
				attribute vec4 aColor; attribute float aSize; uniform float uScale;
				varying vec4 vColor;
				void main(){
					vColor = aColor;
					vec4 mv = modelViewMatrix * vec4(position,1.0);
					gl_Position = projectionMatrix * mv;
					gl_PointSize = aSize * uScale;
				}`,
			fragmentShader: `
				uniform sampler2D uTex; varying vec4 vColor;
				void main(){
					vec4 t = texture2D(uTex, gl_PointCoord);
					gl_FragColor = vec4(vColor.rgb, vColor.a * t.a);
					if (gl_FragColor.a < 0.003) discard;
				}`,
			transparent: true,
			depthWrite: false,
			blending,
		});
		const pts = new THREE.Points(geo, mat);
		pts.frustumCulled = false;
		pts.renderOrder = 10;
		this.scene.add(pts);
		return { geo, mat, pts, parts: [], pos, col, size };
	}

	setScale(s) {
		this.pxScale = s;
		for (const b of this.batches) b.mat.uniforms.uScale.value = s;
	}

	/**
	 * @param {object} o x,y,z world pos; vx,vy,vz velocity; life; size (world units ~ tile); color [r,g,b]; alpha;
	 *   grow; drag; gravity; add (additive blend); fade
	 */
	emit(o) {
		const b = this.batches[o.add === false ? 1 : 0];
		if (b.parts.length >= MAX) return;
		b.parts.push({
			x: o.x, y: o.y, z: o.z, vx: o.vx || 0, vy: o.vy || 0, vz: o.vz || 0, life: o.life, max: o.life,
			size: o.size || 0.2, grow: o.grow || 0, color: o.color || [1, 1, 1], alpha: o.alpha ?? 1,
			drag: o.drag ?? 0, gravity: o.gravity ?? 0, fadeIn: o.fadeIn ?? 0.1,
		});
	}

	burst(n, o, spread = 1, speed = 2) {
		for (let i = 0; i < n; i++) {
			const a = this.rnd() * Math.PI * 2;
			const s = speed * (0.3 + this.rnd() * 0.7);
			this.emit({
				...o,
				vx: Math.cos(a) * s * spread, vz: Math.sin(a) * s * spread, vy: (o.vy ?? 1.2) * (0.3 + this.rnd()),
				life: o.life * (0.6 + this.rnd() * 0.8), size: (o.size || 0.2) * (0.6 + this.rnd() * 0.8),
			});
		}
	}

	update(dt) {
		for (const b of this.batches) {
			const parts = b.parts;
			let n = 0;
			for (let i = 0; i < parts.length; i++) {
				const p = parts[i];
				p.life -= dt;
				if (p.life <= 0) continue;
				parts[n++] = p;
				p.vy -= p.gravity * dt;
				const k = Math.max(0, 1 - p.drag * dt);
				p.vx *= k;
				p.vz *= k;
				p.x += p.vx * dt;
				p.y += p.vy * dt;
				p.z += p.vz * dt;
				p.size += p.grow * dt;
			}
			parts.length = n;
			for (let i = 0; i < n; i++) {
				const p = parts[i];
				const t = p.life / p.max;
				const a = p.alpha * Math.min(1, (1 - t) / (p.fadeIn || 0.001)) * t;
				b.pos[i * 3] = p.x;
				b.pos[i * 3 + 1] = p.y;
				b.pos[i * 3 + 2] = p.z;
				b.col[i * 4] = p.color[0];
				b.col[i * 4 + 1] = p.color[1];
				b.col[i * 4 + 2] = p.color[2];
				b.col[i * 4 + 3] = a;
				b.size[i] = Math.max(0.02, p.size);
			}
			b.geo.setDrawRange(0, n);
			b.geo.attributes.position.needsUpdate = true;
			b.geo.attributes.aColor.needsUpdate = true;
			b.geo.attributes.aSize.needsUpdate = true;
		}
	}

	clear() {
		for (const b of this.batches) b.parts.length = 0;
	}
}
