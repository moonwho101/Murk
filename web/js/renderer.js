/**
 * Murk Three.js Isometric Renderer
 * Handles isometric coordinate projection, sprite UV clipping from 32-bit textures,
 * dynamic atmospheric lighting, dungeon floor/wall tiles, item meshes, and monster/player sprites.
 */

class MurkRenderer {
  constructor(container) {
    this.container = container;
    this.scene = null;
    this.camera = null;
    this.renderer = null;

    this.textures = {};
    this.tileMaterials = {};

    this.dungeonGroup = null;
    this.entityGroup = null;
    this.projectileGroup = null;
    this.effectGroup = null;

    this.tileSize = 64;
    this.cameraTarget = new THREE.Vector3(0, 0, 0);

    this.initThree();
  }

  initThree() {
    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0x06060c);

    const aspect = window.innerWidth / window.innerHeight;
    const viewSize = 750;
    this.camera = new THREE.OrthographicCamera(
      (-viewSize * aspect) / 2,
      (viewSize * aspect) / 2,
      viewSize / 2,
      -viewSize / 2,
      -2000,
      3000
    );

    this.camera.position.set(400, 500, 400);
    this.camera.lookAt(0, 0, 0);

    this.renderer = new THREE.WebGLRenderer({ antialias: false, alpha: false, powerPreference: 'high-performance' });
    this.renderer.setSize(window.innerWidth, window.innerHeight);
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    this.container.appendChild(this.renderer.domElement);

    const ambientLight = new THREE.AmbientLight(0x605570, 0.9);
    this.scene.add(ambientLight);

    const dirLight = new THREE.DirectionalLight(0xffeedd, 0.8);
    dirLight.position.set(300, 600, 200);
    this.scene.add(dirLight);

    this.torchLight = new THREE.PointLight(0xff9933, 1.4, 600, 1.2);
    this.torchLight.position.set(0, 100, 0);
    this.scene.add(this.torchLight);

    this.dungeonGroup = new THREE.Group();
    this.entityGroup = new THREE.Group();
    this.projectileGroup = new THREE.Group();
    this.effectGroup = new THREE.Group();

    this.scene.add(this.dungeonGroup);
    this.scene.add(this.entityGroup);
    this.scene.add(this.projectileGroup);
    this.scene.add(this.effectGroup);

    window.addEventListener('resize', () => this.onResize());
  }

  onResize() {
    const aspect = window.innerWidth / window.innerHeight;
    const viewSize = 750;
    this.camera.left = (-viewSize * aspect) / 2;
    this.camera.right = (viewSize * aspect) / 2;
    this.camera.top = viewSize / 2;
    this.camera.bottom = -viewSize / 2;
    this.camera.updateProjectionMatrix();
    this.renderer.setSize(window.innerWidth, window.innerHeight);
  }

  async loadTextures() {
    const loader = new THREE.TextureLoader();
    const loadTex = (url) => new Promise((resolve, reject) => {
      loader.load(url, (tex) => {
        tex.magFilter = THREE.NearestFilter;
        tex.minFilter = THREE.NearestFilter;
        tex.colorSpace = THREE.SRGBColorSpace;
        resolve(tex);
      }, undefined, reject);
    });

    this.textures.p256 = await loadTex('assets/p256.png');
    this.textures.dungeon = await loadTex('assets/dungeon.png');
    this.textures.dungeon2 = await loadTex('assets/dungeon2.png');
    this.textures.dungeon3 = await loadTex('assets/dungeon3.png');
    this.textures.pan3 = await loadTex('assets/pan3.png');

    console.log('All 32-bit textures loaded successfully.');
  }

  gridToWorld(gx, gy) {
    const x = (gx - gy) * (this.tileSize * 0.7071);
    const z = (gx + gy) * (this.tileSize * 0.7071);
    return { x, y: 0, z };
  }

  screenToGrid(screenX, screenY) {
    const raycaster = new THREE.Raycaster();
    const mouse = new THREE.Vector2(
      (screenX / window.innerWidth) * 2 - 1,
      -(screenY / window.innerHeight) * 2 + 1
    );
    raycaster.setFromCamera(mouse, this.camera);

    const groundPlane = new THREE.Plane(new THREE.Vector3(0, 1, 0), 0);
    const intersectPoint = new THREE.Vector3();
    raycaster.ray.intersectPlane(groundPlane, intersectPoint);

    if (!intersectPoint) return null;

    const factor = this.tileSize * 0.7071;
    const gx = Math.round((intersectPoint.x / factor + intersectPoint.z / factor) / 2);
    const gy = Math.round((intersectPoint.z / factor - intersectPoint.x / factor) / 2);

    return { x: gx, y: gy };
  }

  createSpriteMaterial(tex, x1, y1, x2, y2) {
    const texW = tex.image.width;
    const texH = tex.image.height;

    const u1 = x1 / texW;
    const v1 = 1 - (y2 / texH);
    const u2 = x2 / texW;
    const v2 = 1 - (y1 / texH);

    const cloneTex = tex.clone();
    cloneTex.needsUpdate = true;
    cloneTex.repeat.set(u2 - u1, v2 - v1);
    cloneTex.offset.set(u1, v1);

    return new THREE.MeshBasicMaterial({
      map: cloneTex,
      transparent: true,
      alphaTest: 0.1,
      side: THREE.DoubleSide
    });
  }

  getTileUVRect(code) {
    switch (code) {
      case 1:    return [0, 98, 112, 195];
      case 2:    return [113, 98, 225, 195];
      case 3:    return [226, 98, 338, 195];
      case 4:    return [339, 98, 451, 195];
      case 12:   return [452, 98, 564, 195];
      case 23:   return [565, 98, 677, 195];
      case 34:   return [678, 98, 790, 195];
      case 14:   return [0, 196, 112, 293];
      case 13:   return [113, 196, 225, 293];
      case 24:   return [226, 196, 338, 293];
      case 123:  return [339, 196, 451, 293];
      case 234:  return [452, 196, 564, 293];
      case 134:  return [565, 196, 677, 293];
      case 124:  return [678, 196, 790, 293];
      case 1234: return [0, 0, 112, 97];
      default:   return [0, 0, 112, 97];
    }
  }

  buildDungeonMesh(dungeonSys, currentLevel) {
    while (this.dungeonGroup.children.length > 0) {
      const obj = this.dungeonGroup.children[0];
      this.dungeonGroup.remove(obj);
      if (obj.geometry) obj.geometry.dispose();
      if (obj.material) {
        if (Array.isArray(obj.material)) obj.material.forEach(m => m.dispose());
        else obj.material.dispose();
      }
    }

    const DUNGEONX = dungeonSys.DUNGEONX;
    const DUNGEONY = dungeonSys.DUNGEONY;
    const dungeon = dungeonSys.dungeon;
    const tileType = dungeonSys.tile_type[currentLevel] || 1;
    let baseTex = this.textures.dungeon;
    if (tileType === 2 && this.textures.dungeon2) baseTex = this.textures.dungeon2;
    if (tileType === 3 && this.textures.dungeon3) baseTex = this.textures.dungeon3;

    for (let y = 0; y < DUNGEONY + 1; y++) {
      for (let x = 0; x < DUNGEONX + 1; x++) {
        const cell = dungeon[x] && dungeon[x][y] && dungeon[x][y][currentLevel];
        if (!cell || (cell.type !== 'f' && cell.type !== 's')) continue;

        let patternStr = '';
        if (dungeon[x - 1] && dungeon[x - 1][y] && (dungeon[x - 1][y][currentLevel].type === 'f' || dungeon[x - 1][y][currentLevel].type === 's')) patternStr += '1';
        if (dungeon[x] && dungeon[x][y - 1] && (dungeon[x][y - 1][currentLevel].type === 'f' || dungeon[x][y - 1][currentLevel].type === 's')) patternStr += '2';
        if (dungeon[x + 1] && dungeon[x + 1][y] && (dungeon[x + 1][y][currentLevel].type === 'f' || dungeon[x + 1][y][currentLevel].type === 's')) patternStr += '3';
        if (dungeon[x] && dungeon[x][y + 1] && (dungeon[x][y + 1][currentLevel].type === 'f' || dungeon[x][y + 1][currentLevel].type === 's')) patternStr += '4';
        const patternCode = parseInt(patternStr) || 1234;

        const uvRect = this.getTileUVRect(patternCode);
        const mat = this.createSpriteMaterial(baseTex, uvRect[0], uvRect[1], uvRect[2], uvRect[3]);

        const geo = new THREE.PlaneGeometry(this.tileSize * 1.5, this.tileSize * 1.5);
        const mesh = new THREE.Mesh(geo, mat);
        const worldPos = this.gridToWorld(x, y);
        mesh.position.set(worldPos.x, 0, worldPos.z);
        mesh.rotation.x = -Math.PI / 2;
        mesh.rotation.z = Math.PI / 4;
        mesh.receiveShadow = true;
        this.dungeonGroup.add(mesh);

        if (cell.type === 's') {
          const isUp = cell.item === 1;
          const stairRect = isUp ? [224, 0, 336, 97] : [112, 0, 224, 97];
          const stairMat = this.createSpriteMaterial(this.textures.dungeon, stairRect[0], stairRect[1], stairRect[2], stairRect[3]);
          const sGeo = new THREE.PlaneGeometry(this.tileSize * 1.2, this.tileSize * 1.2);
          const sMesh = new THREE.Mesh(sGeo, stairMat);
          sMesh.position.set(worldPos.x, 2, worldPos.z);
          sMesh.rotation.x = -Math.PI / 2;
          sMesh.rotation.z = Math.PI / 4;
          this.dungeonGroup.add(sMesh);
        }

        if (cell.item && cell.item !== 0 && cell.item !== 1 && cell.item !== 'c') {
          let itemRect = null;
          if (cell.item === 'e') itemRect = [226, 490, 338, 587];
          else if (cell.item === 's') itemRect = [565, 0, 678, 97];
          else if (cell.item === 'r') itemRect = [678, 0, 791, 97];
          else if (cell.item === 'a') itemRect = [450, 0, 562, 97];

          if (itemRect) {
            const itemMat = this.createSpriteMaterial(this.textures.dungeon, itemRect[0], itemRect[1], itemRect[2], itemRect[3]);
            const itemPlane = new THREE.PlaneGeometry(this.tileSize * 0.8, this.tileSize * 0.8);
            const itemMesh = new THREE.Mesh(itemPlane, itemMat);
            itemMesh.position.set(worldPos.x, 15, worldPos.z);
            itemMesh.quaternion.copy(this.camera.quaternion);
            this.dungeonGroup.add(itemMesh);
          }
        }

        if (x === dungeonSys.treasurex && y === dungeonSys.treasurey && currentLevel === dungeonSys.treasurelvl && dungeonSys.foundtreasure === 0) {
          const tMat = this.createSpriteMaterial(this.textures.dungeon, 0, 294, 112, 391);
          const tGeo = new THREE.PlaneGeometry(this.tileSize * 0.9, this.tileSize * 0.9);
          const tMesh = new THREE.Mesh(tGeo, tMat);
          tMesh.position.set(worldPos.x, 20, worldPos.z);
          tMesh.quaternion.copy(this.camera.quaternion);
          this.dungeonGroup.add(tMesh);
        }

        if (x === dungeonSys.treasurex2 && y === dungeonSys.treasurey2 && currentLevel === dungeonSys.treasurelvl2 && dungeonSys.foundtreasure2 === 0) {
          const tMat = this.createSpriteMaterial(this.textures.dungeon, 678, 294, 790, 391);
          const tGeo = new THREE.PlaneGeometry(this.tileSize * 0.9, this.tileSize * 0.9);
          const tMesh = new THREE.Mesh(tGeo, tMat);
          tMesh.position.set(worldPos.x, 20, worldPos.z);
          tMesh.quaternion.copy(this.camera.quaternion);
          this.dungeonGroup.add(tMesh);
        }
      }
    }

    dungeonSys.pits.forEach(pit => {
      if (pit.lvl === currentLevel && pit.frame > 0) {
        const pitRect = [452, 490, 565, 588];
        const pitMat = this.createSpriteMaterial(this.textures.dungeon, pitRect[0], pitRect[1], pitRect[2], pitRect[3]);
        const pitGeo = new THREE.PlaneGeometry(this.tileSize * 1.1, this.tileSize * 1.1);
        const pitMesh = new THREE.Mesh(pitGeo, pitMat);
        const wPos = this.gridToWorld(pit.x, pit.y);
        pitMesh.position.set(wPos.x, 1, wPos.z);
        pitMesh.rotation.x = -Math.PI / 2;
        pitMesh.rotation.z = Math.PI / 4;
        this.dungeonGroup.add(pitMesh);
      }
    });
  }

  updateEntities(entities, playerPos) {
    while (this.entityGroup.children.length > 0) {
      const obj = this.entityGroup.children[0];
      this.entityGroup.remove(obj);
      if (obj.geometry) obj.geometry.dispose();
      if (obj.material) {
        if (Array.isArray(obj.material)) obj.material.forEach(m => m.dispose());
        else obj.material.dispose();
      }
    }

    entities.forEach(ent => {
      if (!ent.active || ent.dead > 64) return;

      const uv = ent.getSpriteUV();
      const mat = this.createSpriteMaterial(this.textures.p256, uv[0], uv[1], uv[2], uv[3]);

      if (ent.hitTimer > 0) {
        mat.color.setRGB(1.5, 0.4, 0.4);
      }

      const geo = new THREE.PlaneGeometry(this.tileSize * 1.1, this.tileSize * 1.1);
      const mesh = new THREE.Mesh(geo, mat);

      const pos = ent.getWorldPosition(this);
      mesh.position.set(pos.x, 24, pos.z);
      mesh.quaternion.copy(this.camera.quaternion);
      this.entityGroup.add(mesh);

      if (ent.protection > 0) {
        const shieldUV = [339, 490, 451, 587];
        const sMat = this.createSpriteMaterial(this.textures.dungeon, shieldUV[0], shieldUV[1], shieldUV[2], shieldUV[3]);
        const sGeo = new THREE.PlaneGeometry(this.tileSize * 1.4, this.tileSize * 1.4);
        const sMesh = new THREE.Mesh(sGeo, sMat);
        sMesh.position.set(pos.x, 28, pos.z);
        sMesh.quaternion.copy(this.camera.quaternion);
        this.entityGroup.add(sMesh);
      }

      if (ent.hasTreasure1 || ent.hasTreasure2) {
        const tUV = ent.hasTreasure1 ? [0, 490, 56, 530] : [113, 490, 169, 587];
        const tMat = this.createSpriteMaterial(this.textures.dungeon, tUV[0], tUV[1], tUV[2], tUV[3]);
        const tGeo = new THREE.PlaneGeometry(this.tileSize * 0.5, this.tileSize * 0.5);
        const tMesh = new THREE.Mesh(tGeo, tMat);
        tMesh.position.set(pos.x, 50, pos.z);
        tMesh.quaternion.copy(this.camera.quaternion);
        this.entityGroup.add(tMesh);
      }
    });
  }

  updateProjectiles(projectiles) {
    while (this.projectileGroup.children.length > 0) {
      const obj = this.projectileGroup.children[0];
      this.projectileGroup.remove(obj);
      if (obj.geometry) obj.geometry.dispose();
      if (obj.material) obj.material.dispose();
    }

    projectiles.forEach(proj => {
      const uv = proj.getSpriteUV();
      const mat = this.createSpriteMaterial(this.textures.p256, uv[0], uv[1], uv[2], uv[3]);
      const geo = new THREE.PlaneGeometry(this.tileSize * 0.8, this.tileSize * 0.8);
      const mesh = new THREE.Mesh(geo, mat);

      const pos = proj.getWorldPosition(this);
      mesh.position.set(pos.x, 20, pos.z);
      mesh.quaternion.copy(this.camera.quaternion);
      this.projectileGroup.add(mesh);
    });
  }

  followPlayer(player) {
    if (!player) return;
    const targetPos = player.getWorldPosition(this);
    this.cameraTarget.lerp(new THREE.Vector3(targetPos.x, 0, targetPos.z), 0.15);

    this.camera.position.set(
      this.cameraTarget.x + 400,
      500,
      this.cameraTarget.z + 400
    );
    this.camera.lookAt(this.cameraTarget.x, 0, this.cameraTarget.z);
    this.torchLight.position.set(targetPos.x, 60, targetPos.z);
  }

  render() {
    this.renderer.render(this.scene, this.camera);
  }
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { MurkRenderer };
}
