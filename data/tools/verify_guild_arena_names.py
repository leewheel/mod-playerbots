#!/usr/bin/env python3
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1] / "sql/characters/base"

for fn in ("playerbots_guild_names.sql", "playerbots_arena_team_names.sql"):
    names = []
    for line in (root / fn).read_text(encoding="utf-8").splitlines():
        if not line.strip().startswith("(NULL"):
            continue
        m = re.search(r"'([^']*)'", line)
        if m:
            names.append(m.group(1))
    dup = len(names) - len(set(names))
    bad = sum(1 for n in names if len(n.encode("utf-8")) > 24)
    non_cn = sum(1 for n in names if not re.search(r"[\u4e00-\u9fff]", n))
    print(f"{fn}: count={len(names)} dup={dup} bad_len={bad} non_cn={non_cn}")
