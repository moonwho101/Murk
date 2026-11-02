const S = Math.SQRT1_2;

/** Keyboard / mouse / gamepad -> per-frame input for the simulation. */
export class Input {
	constructor(canvas) {
		this.canvas = canvas;
		this.keys = new Set();
		this.pressed = new Set();
		this.mouse = { x: 0, y: 0, left: false, right: false, active: false, lastMove: 0 };
		this.enabled = false;
		this.onKey = null;
		this.onWheel = null;
		this.clickTarget = null;

		window.addEventListener('keydown', (e) => {
			if (e.repeat) {
				if (this.enabled && ['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', ' ', 'Tab'].includes(e.key)) e.preventDefault();
				return;
			}
			this.keys.add(e.code);
			this.pressed.add(e.code);
			if (this.enabled && ['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Space', 'Tab'].includes(e.code)) e.preventDefault();
			if (this.onKey) this.onKey(e);
		});
		window.addEventListener('keyup', (e) => this.keys.delete(e.code));
		window.addEventListener('blur', () => {
			this.keys.clear();
			this.mouse.left = this.mouse.right = false;
		});
		canvas.addEventListener('mousemove', (e) => {
			this.mouse.x = e.clientX;
			this.mouse.y = e.clientY;
			this.mouse.active = true;
			this.mouse.lastMove = performance.now();
			if (this.mouse.right) this.clickTarget = { hold: true };
		});
		canvas.addEventListener('mousedown', (e) => {
			this.mouse.x = e.clientX;
			this.mouse.y = e.clientY;
			this.mouse.active = true;
			if (e.button === 0) {
				this.mouse.left = true;
				this.leftPressed = true;
			}
			if (e.button === 2) {
				this.mouse.right = true;
				this.rightPressed = true;
			}
			e.preventDefault();
		});
		window.addEventListener('mouseup', (e) => {
			if (e.button === 0) this.mouse.left = false;
			if (e.button === 2) this.mouse.right = false;
		});
		canvas.addEventListener('contextmenu', (e) => e.preventDefault());
		canvas.addEventListener('dblclick', () => (this.dbl = true));
		canvas.addEventListener(
			'wheel',
			(e) => {
				e.preventDefault();
				if (this.onWheel) this.onWheel(e.deltaY);
			},
			{ passive: false },
		);
	}

	consume(code) {
		const had = this.pressed.has(code);
		this.pressed.delete(code);
		return had;
	}

	endFrame() {
		this.pressed.clear();
		this.dbl = false;
	}

	/**
	 * @returns input object for World.update
	 */
	poll(view, world) {
		const k = this.keys;
		let sx = 0;
		let sy = 0;
		if (k.has('KeyA') || k.has('ArrowLeft')) sx -= 1;
		if (k.has('KeyD') || k.has('ArrowRight')) sx += 1;
		if (k.has('KeyW') || k.has('ArrowUp')) sy += 1;
		if (k.has('KeyS') || k.has('ArrowDown')) sy -= 1;

		const pad = this.gamepad();
		let padFire = false;
		let padAim = null;
		if (pad) {
			const ax = Math.abs(pad.axes[0]) > 0.2 ? pad.axes[0] : 0;
			const ay = Math.abs(pad.axes[1]) > 0.2 ? pad.axes[1] : 0;
			sx += ax;
			sy -= ay;
			const rx = Math.abs(pad.axes[2]) > 0.25 ? pad.axes[2] : 0;
			const ry = Math.abs(pad.axes[3]) > 0.25 ? pad.axes[3] : 0;
			if (rx || ry) {
				const sxr = rx;
				const syr = -ry;
				padAim = Math.atan2((-sxr - syr) * S, (sxr - syr) * S);
			}
			padFire = !!(pad.buttons[7]?.pressed || pad.buttons[0]?.pressed || pad.buttons[5]?.pressed);
			if (pad.buttons[2]?.pressed) this.pressed.add('KeyR');
			if (pad.buttons[1]?.pressed) this.pressed.add('KeyE');
			if (pad.buttons[3]?.pressed) this.pressed.add('KeyT');
		}

		let moveX = (sx - sy) * S;
		let moveY = (-sx - sy) * S;
		const len = Math.hypot(moveX, moveY);
		if (len > 1) {
			moveX /= len;
			moveY /= len;
		}

		const p = world.player;
		const manual = Math.hypot(moveX, moveY) > 0.05;
		const out = { moveX, moveY, aim: null, fire: false, ring: false, drop: false, interact: false };
		const mouseRecent = this.mouse.active && performance.now() - this.mouse.lastMove < 4000;
		if (mouseRecent || this.mouse.left || this.mouse.right) {
			const g = view.screenToGrid(this.mouse.x, this.mouse.y, 0.45);
			if (g && Math.hypot(g.x - p.x, g.y - p.y) > 0.25) out.aim = Math.atan2(g.y - p.y, g.x - p.x);
			this.mouseGrid = g;
		}
		if (padAim != null) out.aim = padAim;

		// Left button: run along the nearest grid axis toward the cursor while held. Right button / Ctrl fire.
		this.leftPressed = false;
		if (this.mouse.left && !manual && p.alive && out.aim != null) {
			const dx = Math.cos(out.aim);
			const dy = Math.sin(out.aim);
			if (Math.abs(dx) >= Math.abs(dy)) {
				out.moveX = Math.sign(dx);
				out.moveY = 0;
			} else {
				out.moveX = 0;
				out.moveY = Math.sign(dy);
			}
		}
		const tapFire = this.rightPressed;
		this.rightPressed = false;
		if (this.mouse.right || tapFire || k.has('ControlLeft') || k.has('ControlRight') || padFire) out.fire = true;
		if (this.consume('KeyR')) out.ring = true;
		if (this.consume('KeyT')) out.drop = true;
		if (this.consume('KeyE') || this.consume('ShiftLeft') || this.consume('ShiftRight') || this.dbl) out.interact = true;
		if (len > 0.05) out.aimFromMove = true;
		// keyboard-only: shoot where you walk unless the mouse recently moved
		if (!mouseRecent && padAim == null) out.aim = null;
		return out;
	}

	gamepad() {
		if (!navigator.getGamepads) return null;
		for (const g of navigator.getGamepads()) if (g && g.connected) return g;
		return null;
	}
}
