// Seedable PRNG (mulberry32) so dungeon generation and simulation are testable.
export class Rng {
	constructor(seed = Date.now() >>> 0) {
		this.s = seed >>> 0;
	}
	next() {
		let t = (this.s += 0x6d2b79f5);
		t = Math.imul(t ^ (t >>> 15), t | 1);
		t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
		return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
	}
	/** integer in [0, n) */
	int(n) {
		return Math.floor(this.next() * n);
	}
	/** integer in [a, b] inclusive */
	range(a, b) {
		return a + this.int(b - a + 1);
	}
	float(a = 0, b = 1) {
		return a + this.next() * (b - a);
	}
	chance(p) {
		return this.next() < p;
	}
	pick(arr) {
		return arr[this.int(arr.length)];
	}
	shuffle(arr) {
		for (let i = arr.length - 1; i > 0; i--) {
			const j = this.int(i + 1);
			[arr[i], arr[j]] = [arr[j], arr[i]];
		}
		return arr;
	}
}
