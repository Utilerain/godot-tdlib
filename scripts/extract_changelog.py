#!/usr/bin/env python3
"""Extracts the CHANGELOG.md section for a given tag (Keep a Changelog format).

Usage: python3 scripts/extract_changelog.py <tag>
Prints the section body. If no section matches the tag, prints a fallback note.
"""

import re
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: extract_changelog.py <tag>", file=sys.stderr)
        return 1
    tag = sys.argv[1]
    changelog = (Path(__file__).resolve().parent.parent / "CHANGELOG.md").read_text(encoding="utf-8")
    pattern = rf"^## \[{re.escape(tag)}\][^\n]*\n(.*?)(?=^## \[|\Z)"
    match = re.search(pattern, changelog, re.M | re.S)
    if match is None:
        print(f"warning: no CHANGELOG.md section for [{tag}], using a fallback note", file=sys.stderr)
        print(f"Release {tag}. See CHANGELOG.md for the full list of changes.")
        return 0
    print(match.group(1).strip())
    return 0


if __name__ == "__main__":
    sys.exit(main())
