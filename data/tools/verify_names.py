#!/usr/bin/env python3
from pathlib import Path
import re

path = Path(__file__).resolve().parents[1] / "sql/characters/base/playerbots_names.sql"
pairs = set()
dup = 0
bad_len = 0
for line in path.read_text(encoding="utf-8").splitlines():
    m = re.match(r"\((\d+),'([^']*)',(\d+)\)", line.strip().rstrip(",").rstrip(";"))
    if not m:
        continue
    name, gender = m.group(2), int(m.group(3))
    pair = (name, gender)
    if pair in pairs:
        dup += 1
    pairs.add(pair)
    if len(name.encode("utf-8")) > 12:
        bad_len += 1
print(f"rows={len(pairs)} dup_pairs={dup} bad_len={bad_len}")
