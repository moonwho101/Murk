"""Convert the original levels*.dat episode files to JSON for the web port.

Usage:  python tools/convert_levels.py
"""

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "web" / "src" / "data" / "episodes.json"

FIELDS = [
    "gametype",
    "beholders", "bspeed", "deathorb", "dspeed", "skeletons", "sspeed",
    "mouther", "mspeed", "ogre", "ospeed",
    "fighter", "fteam", "fspeed", "cleric", "cteam", "cspeed",
    "thief", "tteam", "tspeed", "wizard", "wteam", "wspeed",
    "dungeonx", "dungeony", "dungeonlvl", "pits", "maxpit",
]
RECORD_LINES = 2 + len(FIELDS)

FILES = {"fighter": "levelsb.dat", "thief": "levelst.dat", "wizard": "levelsw.dat", "cleric": "levelsc.dat"}


def parse(path):
    lines = [l.rstrip("\r") for l in path.read_text(encoding="latin1").split("\n")]
    episodes = []
    for i in range(len(lines) // RECORD_LINES):
        rec = lines[i * RECORD_LINES:(i + 1) * RECORD_LINES]
        ep = {"text": rec[0], "title": rec[1]}
        for key, line in zip(FIELDS, rec[2:]):
            m = re.match(r"\s*(-?\d+)", line)
            ep[key] = int(m.group(1)) if m else 0
        episodes.append(ep)
    return episodes


if __name__ == "__main__":
    data = {hero: parse(ROOT / "bin" / f) for hero, f in FILES.items()}
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(data, indent=0, separators=(",", ":")))
    for hero, eps in data.items():
        print(hero, len(eps), "episodes")
