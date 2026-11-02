/**
 * Murk audio: Web Audio playback of the original WAV effects plus an ambient loop.
 */

class MurkAudio {
  constructor() {
    this.ctx = null;
    this.buffers = {};
    this.muted = false;
    this.loading = null;
    this.ambient = null;
    this.dripTimer = null;
    // Names as stored on disk (the server may be case sensitive); lookups are case-insensitive.
    this.files = [
      'alive.wav', 'back.wav', 'bsword.wav', 'button.wav', 'dead.wav', 'deathorb.wav', 'die.wav',
      'DRAGON.WAV', 'Drip2.wav', 'ewizard.wav', 'fireball.wav', 'impact.wav', 'item.wav', 'mouther.wav',
      'orb.wav', 'ouch.wav', 'Protection.wav', 'Skeleton.wav', 'Spawn.wav', 'tcleric.wav', 'throw.wav',
      'throw2.wav', 'treasure.wav', 'tsword.wav', 'walk.wav', 'win.wav'
    ];
  }

  init() {
    if (!this.ctx) {
      const AudioCtx = window.AudioContext || window.webkitAudioContext;
      if (!AudioCtx) return;
      this.ctx = new AudioCtx();
      this.master = this.ctx.createGain();
      this.master.gain.value = this.muted ? 0 : 1;
      this.master.connect(this.ctx.destination);
    }
    if (this.ctx.state === 'suspended') this.ctx.resume();
    if (!this.loading) this.loading = this.preload();
  }

  async preload() {
    await Promise.all(this.files.map(async (file) => {
      try {
        const res = await fetch(`assets/sound/${file}`);
        if (!res.ok) return;
        const data = await res.arrayBuffer();
        this.buffers[file.toLowerCase()] = await this.ctx.decodeAudioData(data);
      } catch (err) {
        console.warn('Could not load sound', file, err);
      }
    }));
    this.startAmbience();
  }

  startAmbience() {
    if (this.ambient || !this.ctx || !this.buffers['back.wav']) return;
    const src = this.ctx.createBufferSource();
    src.buffer = this.buffers['back.wav'];
    src.loop = true;
    const gain = this.ctx.createGain();
    gain.gain.value = 0.18;
    src.connect(gain);
    gain.connect(this.master);
    src.start(0);
    this.ambient = src;

    const drip = () => {
      this.play('drip2.wav', 0.12);
      this.dripTimer = setTimeout(drip, 4000 + Math.random() * 9000);
    };
    this.dripTimer = setTimeout(drip, 3000);
  }

  toggleMute() {
    this.muted = !this.muted;
    if (this.master) this.master.gain.value = this.muted ? 0 : 1;
    return this.muted;
  }

  play(name, volume = 0.5, rate = 1) {
    if (this.muted || !this.ctx) return;
    const buf = this.buffers[name.toLowerCase()];
    if (!buf) return;
    try {
      const source = this.ctx.createBufferSource();
      source.buffer = buf;
      source.playbackRate.value = rate;
      const gain = this.ctx.createGain();
      gain.gain.value = volume;
      source.connect(gain);
      gain.connect(this.master);
      source.start(0);
    } catch (e) {
      // playback is best effort
    }
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { MurkAudio };
}
