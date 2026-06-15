#!/usr/bin/env python3
import re
from pathlib import Path

path = Path(__file__).resolve().parents[1] / "sql/playerbots/base/ai_playerbot_texts.sql"
for line in path.read_text(encoding="utf-8").splitlines():
    if not line.strip().startswith("("):
        continue
    # id, name, text, say, reply, loc1..loc8
    parts = line.strip().rstrip(",").split(", ", 2)
    if len(parts) < 3:
        continue
    # crude: find loc4 as 4th quoted field after reply_type
    m = re.match(
        r"\((\d+), '([^']*)', '([^']*)', (\d+), (\d+), '([^']*)', '([^']*)', '([^']*)', '([^']*)'",
        line.strip().rstrip(","),
    )
    if not m:
        continue
    id_, name, text, loc4 = m.group(1), m.group(2), m.group(3), m.group(8)
    if not loc4 and re.search(r"[A-Za-z]{3,}", text):
        print(f"id={id_} name={name} text={text[:80]}")
