"""Convert the original 8-bit Murk artwork into 32-bit RGBA PNGs for the three.js port.

Usage:  python tools/convert_assets.py

Outputs go to web/public/assets/.  Requires: pillow, numpy.

Pixel-art sheets are upscaled 2x with the Scale2x (EPX) algorithm on the palette
indices (crisp diagonals, no new colours), converted to RGBA, and the RGB of
fully transparent pixels is "bled" from neighbours so linear texture filtering
never produces halos.
"""

import json
import shutil
import zipfile
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
ART = ROOT / "artwork"
SRC_SOUND = ROOT / "sound"
OUT = ROOT / "web" / "public" / "assets"
TMP = ROOT / "tools" / ".tmp"

TRANSPARENT_INDEX = 255


def load_indexed(path):
    im = Image.open(path)
    if im.mode != "P":
        raise ValueError(f"{path} is not paletted")
    pal = np.array(im.getpalette()[:768], dtype=np.uint8).reshape(-1, 3)
    if len(pal) < 256:
        pal = np.vstack([pal, np.zeros((256 - len(pal), 3), np.uint8)])
    return np.array(im), pal


def scale2x_indices(p):
    h, w = p.shape
    pp = np.pad(p, 1, mode="edge")
    b = pp[0:h, 1:w + 1]
    d = pp[1:h + 1, 0:w]
    f = pp[1:h + 1, 2:w + 2]
    hh = pp[2:h + 2, 1:w + 1]
    e0 = np.where((b == d) & (b != f) & (d != hh), d, p)
    e1 = np.where((b == f) & (b != d) & (f != hh), f, p)
    e2 = np.where((d == hh) & (d != b) & (hh != f), d, p)
    e3 = np.where((hh == f) & (d != hh) & (b != f), f, p)
    out = np.empty((h * 2, w * 2), dtype=p.dtype)
    out[0::2, 0::2] = e0
    out[0::2, 1::2] = e1
    out[1::2, 0::2] = e2
    out[1::2, 1::2] = e3
    return out


def indices_to_rgba(idx, pal, transparent):
    rgba = np.zeros(idx.shape + (4,), np.uint8)
    rgba[..., :3] = pal[idx]
    rgba[..., 3] = 255
    if transparent is not None:
        rgba[idx == transparent, 3] = 0
    return rgba


def bleed(rgba, iterations=3):
    """Spread colour into transparent pixels (alpha stays 0)."""
    rgb = rgba[..., :3].astype(np.float32)
    known = rgba[..., 3] > 0
    for _ in range(iterations):
        acc = np.zeros_like(rgb)
        cnt = np.zeros(known.shape, np.float32)
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if dx == 0 and dy == 0:
                    continue
                k = np.roll(np.roll(known, dy, 0), dx, 1)
                c = np.roll(np.roll(rgb, dy, 0), dx, 1)
                acc += c * k[..., None]
                cnt += k
        fill = (~known) & (cnt > 0)
        rgb[fill] = acc[fill] / cnt[fill][..., None]
        known = known | fill
    out = rgba.copy()
    out[..., :3] = np.clip(rgb + 0.5, 0, 255).astype(np.uint8)
    return out


def paletted_png_to_rgba(path, upscale=True):
    im = Image.open(path)
    tr = im.info.get("transparency", 0)
    if isinstance(tr, bytes):
        tr = tr.index(0)
    idx = np.array(im)
    pal = np.array(im.getpalette()[:768], dtype=np.uint8).reshape(-1, 3)
    if len(pal) < 256:
        pal = np.vstack([pal, np.zeros((256 - len(pal), 3), np.uint8)])
    if upscale:
        idx = scale2x_indices(idx)
    return bleed(indices_to_rgba(idx, pal, tr))


def save(rgba, rel):
    dest = OUT / rel
    dest.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(rgba, "RGBA").save(dest, optimize=True)
    print("  wrote", rel, rgba.shape[1], "x", rgba.shape[0])


def alpha_bbox(rgba):
    ys, xs = np.nonzero(rgba[..., 3] > 8)
    if len(xs) == 0:
        return None
    return int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1


def quad_texture(rgba, quad, size, inset=0.0):
    """Rectify a quadrilateral (UL, LL, LR, UR in source px) into a texture."""
    q = np.array(quad, dtype=np.float64)
    if inset:
        c = q.mean(axis=0)
        q = c + (q - c) * (1 - inset)
    im = Image.fromarray(rgba[..., :3], "RGB")
    data = tuple(float(v) for v in q.flatten())
    return np.array(im.transform(size, Image.QUAD, data, Image.BICUBIC))


# --------------------------------------------------------------------------- #

def convert_sprites():
    print("hero / monster sprites")
    idx, pal = load_indexed(ART / "p256.bmp")
    rgba = bleed(indices_to_rgba(scale2x_indices(idx), pal, TRANSPARENT_INDEX))
    save(rgba, "sprites/heroes.png")

    # Per-block foot line so billboards stand on the floor.
    blocks = {}
    names = ["fighter", "beholder", "thief", "wizard", "cleric", "missiles",
             "skeleton", "mouther", "ogre"]
    for i, name in enumerate(names):
        bottoms, tops = [], []
        for r in range(2):
            for c in range(12):
                x0, y0 = c * 128, (i * 2 + r) * 128
                bb = alpha_bbox(rgba[y0:y0 + 128, x0:x0 + 128])
                if bb:
                    tops.append(bb[1])
                    bottoms.append(bb[3])
        blocks[name] = {
            "row": i * 2,
            "foot": float(np.median(bottoms)) / 128.0 if bottoms else 1.0,
            "top": float(np.median(tops)) / 128.0 if tops else 0.0,
        }
    (OUT / "sprites").mkdir(parents=True, exist_ok=True)
    (OUT / "sprites" / "heroes.json").write_text(
        json.dumps({"cell": 128, "cols": 12, "blocks": blocks}, indent=1))


ITEM_CELLS = {
    "stairsDown": (112, 0), "stairsUp": (224, 0), "armour": (450, 0),
    "scroll": (565, 0), "ring": (678, 0),
    "chestRed": (0, 294), "chestBlue": (678, 294),
    "potion": (226, 490), "bubble": (339, 490),
}


def convert_items():
    print("items")
    idx, pal = load_indexed(ART / "dungeon.bmp")
    full = bleed(indices_to_rgba(scale2x_indices(idx), pal, TRANSPARENT_INDEX))
    names = list(ITEM_CELLS)
    cw, ch = 113 * 2, 98 * 2
    strip = np.zeros((ch, cw * len(names), 4), np.uint8)
    meta = {"cellW": cw, "cellH": ch, "items": {}}
    for n, name in enumerate(names):
        x, y = ITEM_CELLS[name]
        cell = full[y * 2:y * 2 + ch, x * 2:x * 2 + cw]
        strip[:cell.shape[0], n * cw:n * cw + cell.shape[1]] = cell
        bb = alpha_bbox(cell) or (0, 0, cw, ch)
        meta["items"][name] = {"index": n, "bbox": list(bb)}
    save(strip, "sprites/items.png")
    (OUT / "sprites" / "items.json").write_text(json.dumps(meta, indent=1))

    hs = np.array(Image.open(OUT / "sprites" / "heroes.png"))
    save(hs[1152 * 2:1152 * 2 + 128, 0:128], "sprites/splat.png")


THEMES = {
    "cave": "DUNGEON1.SPR.png",
    "ash": "DUNGEON2.SPR.png",
    "lava": "DUNGEON3.SPR.png",
    "stone": "DUNGEONB.SPR.png",
    "moss": "DUNGEONG.SPR.png",
    "ember": "DUNGEONR.SPR.png",
    "frost": "DUNGEONU.SPR.png",
    "bone": "DUNGEONW.SPR.png",
}


def convert_themes():
    print("themes")
    TMP.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(ART / "DUNGEONPNG.zip") as z:
        z.extractall(TMP)
    meta = {}
    S = 2
    for name, fname in THEMES.items():
        rgba = paletted_png_to_rgba(TMP / fname)
        save(rgba, f"themes/{name}_sheet.png")

        # floor diamond in cell (0,0): top(32,32) left(0,48) bottom(32,64) right(64,48)
        quad = [(32 * S, 32 * S), (1 * S, 48 * S), (32 * S, 63 * S), (63 * S, 48 * S)]
        floor = quad_texture(rgba, quad, (256, 256), inset=0.04)
        Image.fromarray(floor, "RGB").save(OUT / "themes" / f"{name}_floor.png")

        # north-east wall face in cell (1,0): base (96,32)->(128,48)
        x0 = 64 * S
        sub = rgba[0:64 * S, x0 + 36 * S:x0 + 62 * S, 3] > 0
        heights = []
        for c in range(sub.shape[1]):
            if sub[:, c].any():
                px = 36 * S + c  # x within the cell, in 2x px
                base_y = 32 * S + (px - 32 * S) / 2.0
                heights.append(base_y - np.nonzero(sub[:, c])[0].min())
        dy = (float(np.median(heights)) if heights else 24 * S) - 3
        wall_quad = [
            (x0 + 33 * S, 32.5 * S - dy), (x0 + 33 * S, 32.5 * S),
            (x0 + 63 * S, 47.5 * S), (x0 + 63 * S, 47.5 * S - dy),
        ]
        wall = quad_texture(rgba, wall_quad, (256, 128))
        Image.fromarray(wall, "RGB").save(OUT / "themes" / f"{name}_wall.png")
        meta[name] = {"avgFloor": [int(v) for v in floor.reshape(-1, 3).mean(axis=0)]}
        print(f"  theme {name}: wall height {dy / S:.1f}px")

    # the original mossy dungeon floor from dungeon.bmp
    idx, pal = load_indexed(ART / "dungeon.bmp")
    full = bleed(indices_to_rgba(scale2x_indices(idx), pal, TRANSPARENT_INDEX))
    quad = [(52 * S, 43 * S), (5 * S, 67 * S), (52 * S, 91 * S), (98 * S, 67 * S)]
    floor = quad_texture(full, quad, (256, 256), inset=0.05)
    Image.fromarray(floor, "RGB").save(OUT / "themes" / "murk_floor.png")
    shutil.copy(OUT / "themes" / "stone_wall.png", OUT / "themes" / "murk_wall.png")
    meta["murk"] = {"avgFloor": [int(v) for v in floor.reshape(-1, 3).mean(axis=0)]}
    (OUT / "themes" / "themes.json").write_text(json.dumps(meta, indent=1))
    shutil.rmtree(TMP, ignore_errors=True)


def convert_ui():
    print("ui art")
    for src, dst, tr in (("murk256.bmp", "ui/splash.png", None),
                         ("SCROLL.bmp", "ui/scroll.png", TRANSPARENT_INDEX)):
        idx, pal = load_indexed(ART / src)
        rgba = indices_to_rgba(idx, pal, tr)
        im = Image.fromarray(rgba, "RGBA").resize((idx.shape[1] * 2, idx.shape[0] * 2), Image.LANCZOS)
        save(np.array(im), dst)

    # Character-select portraits (highlighted variant = right-hand column)
    idx, pal = load_indexed(ART / "select.bmp")
    bg = int(idx[0, 0])
    for i, name in enumerate(["thief", "cleric", "wizard", "fighter"]):
        sub = idx[i * 325:(i + 1) * 325, 400:800]
        rgba = indices_to_rgba(sub, pal, bg)
        im = Image.fromarray(rgba, "RGBA").resize((800, 650), Image.LANCZOS)
        save(np.array(im), f"ui/portrait_{name}.png")

    for src, dst in (("pan3.bmp", "ui/panel.png"), ("dungeon2.bmp", "art/dungeon2.png"),
                     ("dungeon3.bmp", "art/dungeon3.png")):
        idx, pal = load_indexed(ART / src)
        save(bleed(indices_to_rgba(idx, pal, TRANSPARENT_INDEX)), dst)

    save(np.array(Image.open(ART / "elixir.png").convert("RGBA")), "ui/elixir.png")
    for jpg in ("beholder.jpg", "deathorb.jpg", "gibberingmouther.jpg"):
        save(np.array(Image.open(ART / jpg).convert("RGBA")), "art/" + jpg.replace(".jpg", ".png"))


def copy_sounds():
    print("sounds")
    dest = OUT / "sound"
    dest.mkdir(parents=True, exist_ok=True)
    for f in SRC_SOUND.iterdir():
        if f.suffix.lower() == ".wav":
            shutil.copy(f, dest / f.name.lower())
    icon = Image.open(ART / "icon3.png").convert("RGBA").resize((128, 128), Image.LANCZOS)
    icon.save(ROOT / "web" / "public" / "favicon.png", optimize=True)


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    convert_sprites()
    convert_items()
    convert_themes()
    convert_ui()
    copy_sounds()
    print("done")
