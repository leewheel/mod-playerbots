#!/usr/bin/env python3
import re
from pathlib import Path

path = Path(__file__).resolve().parents[1] / "sql/playerbots/base/ai_playerbot_texts.sql"
for line in path.read_text(encoding="utf-8").splitlines():
    if not line.strip().startswith("("):
        continue
    m = re.match(
        r"\((\d+), '([^']*)', '([^']*)', (\d+), (\d+), '([^']*)', '([^']*)', '([^']*)', '([^']*)'",
        line.strip().rstrip(","),
    )
    if not m:
        continue
    id_, name, text, loc4 = m.group(1), m.group(2), m.group(3), m.group(8)
    has_cn = bool(re.search(r"[\u4e00-\u9fff]", text))
    if not has_cn:
        print(f"id={id_} name={name} text={text}")
