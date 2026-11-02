/**
 * Murk Audio Manager
 * Uses Web Audio API to play original Murk sounds with volume controls and spatial effects.
 */

class MurkAudio {
  constructor() {
    this.ctx = null;
    this.buffers = {};
    this.isMuted = false;
    this.soundList = [
      'walk.wav', 'back.wav', 'drip2.wav', 'orb.wav', 'treasure.wav',
      'dead.wav', 'win.wav', 'item.wav', 'throw.wav', 'throw2.wav',
      'impact.wav', 'spawn.wav', 'die.wav', 'mouther.wav', 'dragon.wav',
      'alive.wav', 'skeleton.wav', 'deathorb.wav', 'fireball.wav',
      'ouch.wav', 'button.wav', 'tsword.wav', 'bsword.wav', 'ewizard.wav',
      'tcleric.wav', 'protection.wav'
    ];
  }

  init() {
    if (!this.ctx) {
      const AudioCtx = window.AudioContext || window.webkitAudioContext;
      if (AudioCtx) {
        this.ctx = new AudioCtx();
      }
    }
    if (this.ctx && this.ctx.state === 'suspended') {
      this.ctx.resume();
    }
  }

  async preload() {
    this.init();
    if (!this.ctx) return;

    for (const name of this.soundList) {
      try {
        const res = await fetch(`assets/sound/${name}`);
        if (!res.ok) continue;
        const arrayBuf = await res.arrayBuffer();
        this.buffers[name] = await this.ctx.decodeAudioData(arrayBuf);
      } catch (err) {
        // Silently continue if audio decoding fails
      }
    }
  }

  play(name, volume = 0.5) {
    if (this.isMuted || !this.ctx || !this.buffers[name]) return;

    try {
      const source = this.ctx.createBufferSource();
      source.buffer = this.buffers[name];
      const gainNode = this.ctx.createGain();
      gainNode.gain.value = volume;
      source.connect(gainNode);
      gainNode.connect(this.ctx.destination);
      source.start(0);
    } catch (e) {
      // Audio playback exception ignored
    }
  }
}
