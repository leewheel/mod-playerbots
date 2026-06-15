#!/usr/bin/env python3
"""Find likely player-visible English strings in src/."""
import re
from pathlib import Path

SRC = Path(__file__).resolve().parents[2] / "src"
# Skip LOG_, comments-only patterns; focus on quoted strings with Latin letters
PATTERNS = [
    re.compile(r'(TellMaster|TellError|TellPlayer|SendSysMessage|PSendSysMessage|Whisper|Yell|Say)\([^)]*"([A-Za-z][^"]{3,})"'),
    re.compile(r'out\s*<<\s*"([A-Za-z][^"]{3,})"'),
    re.compile(r'GetBotTextOrDefault\(\s*"[^"]+",\s*"([A-Za-z][^"]{3,})"'),
    re.compile(r'new TellMasterAction\([^,]+,\s*"([A-Za-z][^"]{3,})"'),
]

skip_substrings = {"DEBUG", "http", "LANG_", "cast ", "castnc", "Playerbot", "World", "ERROR"}

for path in sorted(SRC.rglob("*.cpp")):
    text = path.read_text(encoding="utf-8", errors="replace")
    for i, line in enumerate(text.splitlines(), 1):
        if line.strip().startswith("//") or line.strip().startswith("*"):
            continue
        if not re.search(r"[A-Za-z]{4,}", line):
            continue
        if re.search(r"[\u4e00-\u9fff]", line):
            # mixed line - still report if lots of English words
            en_words = len(re.findall(r"\b[A-Za-z]{4,}\b", line))
            cn = len(re.findall(r"[\u4e00-\u9fff]", line))
            if cn >= en_words:
                continue
        for pat in PATTERNS:
            m = pat.search(line)
            if m:
                s = m.group(1) if m.lastindex else m.group(0)
                if any(x in line for x in skip_substrings):
                    continue
                print(f"{path.relative_to(SRC.parent)}:{i}: {line.strip()[:120]}")
                break
