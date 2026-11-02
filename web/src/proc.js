import * as THREE from 'three';

function canvas(w, h) {
	const c = document.createElement('canvas');
	c.width = w;
	c.height = h;
	return [c, c.getContext('2d')];
}

function toTexture(c, srgb = true) {
	const t = new THREE.CanvasTexture(c);
	if (srgb) t.colorSpace = THREE.SRGBColorSpace;
	t.anisotropy = 4;
	return t;
}

export function glowTexture(color = '#ffffff', size = 128, hard = 0) {
	const [c, g] = canvas(size, size);
	const r = size / 2;
	const grad = g.createRadialGradient(r, r, 0, r, r, r);
	grad.addColorStop(0, color);
	grad.addColorStop(Math.max(0, hard), color);
	grad.addColorStop(1, 'rgba(0,0,0,0)');
	g.fillStyle = grad;
	g.fillRect(0, 0, size, size);
	return toTexture(c);
}

export function softDot() {
	const [c, g] = canvas(64, 64);
	const grad = g.createRadialGradient(32, 32, 0, 32, 32, 32);
	grad.addColorStop(0, 'rgba(255,255,255,1)');
	grad.addColorStop(0.35, 'rgba(255,255,255,0.55)');
	grad.addColorStop(1, 'rgba(255,255,255,0)');
	g.fillStyle = grad;
	g.fillRect(0, 0, 64, 64);
	return toTexture(c);
}

export function shadowBlob() {
	const [c, g] = canvas(128, 128);
	const grad = g.createRadialGradient(64, 64, 0, 64, 64, 64);
	grad.addColorStop(0, 'rgba(0,0,0,0.75)');
	grad.addColorStop(0.55, 'rgba(0,0,0,0.4)');
	grad.addColorStop(1, 'rgba(0,0,0,0)');
	g.fillStyle = grad;
	g.fillRect(0, 0, 128, 128);
	return toTexture(c);
}

export function flameTexture() {
	const [c, g] = canvas(64, 128);
	const grad = g.createRadialGradient(32, 88, 2, 32, 76, 40);
	grad.addColorStop(0, 'rgba(255,250,200,1)');
	grad.addColorStop(0.25, 'rgba(255,190,70,0.95)');
	grad.addColorStop(0.6, 'rgba(255,90,20,0.5)');
	grad.addColorStop(1, 'rgba(255,40,0,0)');
	g.fillStyle = grad;
	g.beginPath();
	g.moveTo(32, 4);
	g.bezierCurveTo(52, 50, 62, 70, 32, 124);
	g.bezierCurveTo(2, 70, 12, 50, 32, 4);
	g.fill();
	return toTexture(c);
}

/** Glowing runic circle used for starting pads (flat on the floor, top-down). */
export function padTexture(color) {
	const s = 256;
	const [c, g] = canvas(s, s);
	g.translate(s / 2, s / 2);
	const glow = g.createRadialGradient(0, 0, 10, 0, 0, s / 2);
	glow.addColorStop(0, color + '66');
	glow.addColorStop(0.7, color + '22');
	glow.addColorStop(1, color + '00');
	g.fillStyle = glow;
	g.fillRect(-s / 2, -s / 2, s, s);
	g.strokeStyle = color;
	g.lineWidth = 6;
	g.shadowColor = color;
	g.shadowBlur = 14;
	g.beginPath();
	g.arc(0, 0, 100, 0, Math.PI * 2);
	g.stroke();
	g.lineWidth = 3;
	g.beginPath();
	g.arc(0, 0, 78, 0, Math.PI * 2);
	g.stroke();
	g.lineWidth = 4;
	for (let i = 0; i < 12; i++) {
		const a = (i / 12) * Math.PI * 2;
		g.beginPath();
		g.moveTo(Math.cos(a) * 80, Math.sin(a) * 80);
		g.lineTo(Math.cos(a) * 98, Math.sin(a) * 98);
		g.stroke();
	}
	g.beginPath();
	for (let i = 0; i < 5; i++) {
		const a = (i / 5) * Math.PI * 2 - Math.PI / 2;
		const x = Math.cos(a) * 70;
		const y = Math.sin(a) * 70;
		if (i === 0) g.moveTo(x, y);
		else g.lineTo(x, y);
	}
	g.closePath();
	g.stroke();
	return toTexture(c);
}

export function pitTexture() {
	const s = 256;
	const [c, g] = canvas(s, s);
	const grad = g.createRadialGradient(s / 2, s / 2, 10, s / 2, s / 2, s / 2 - 6);
	grad.addColorStop(0, '#000');
	grad.addColorStop(0.7, '#050205');
	grad.addColorStop(0.9, '#2a1410');
	grad.addColorStop(1, 'rgba(60,30,20,0)');
	g.fillStyle = grad;
	g.beginPath();
	g.arc(s / 2, s / 2, s / 2 - 4, 0, Math.PI * 2);
	g.fill();
	g.strokeStyle = 'rgba(160,90,50,0.7)';
	g.lineWidth = 5;
	g.beginPath();
	g.arc(s / 2, s / 2, s / 2 - 22, 0, Math.PI * 2);
	g.stroke();
	return toTexture(c);
}

export function stairsGlowTexture(color) {
	return glowTexture(color, 128, 0.1);
}

/** Wide, dark stone slab used as the tops of wall blocks. */
export function wallTopTexture() {
	const [c, g] = canvas(128, 128);
	g.fillStyle = '#4a4a4f';
	g.fillRect(0, 0, 128, 128);
	for (let i = 0; i < 700; i++) {
		const v = 50 + Math.random() * 40;
		g.fillStyle = `rgba(${v},${v},${v + 6},0.5)`;
		g.fillRect(Math.random() * 128, Math.random() * 128, 2 + Math.random() * 5, 2 + Math.random() * 5);
	}
	return toTexture(c);
}

export function iconCanvas(img, sx, sy, sw, sh, size = 48) {
	const [c, g] = canvas(size, size);
	const k = Math.min(size / sw, size / sh);
	g.imageSmoothingQuality = 'high';
	g.drawImage(img, sx, sy, sw, sh, (size - sw * k) / 2, (size - sh * k) / 2, sw * k, sh * k);
	return c;
}
