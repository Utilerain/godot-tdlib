#!/usr/bin/env python3
"""Checks that TDLib version references are consistent across the repository.

Sources checked (all must report the same version):
  * SConstruct                              -> tdlib_version
  * tests/test_runner.gd                    -> TDLIB_VERSION
  * .github/workflows/ci.yml                -> libtdjson.so.X.Y.Z
  * addons/godot-tdlib/telegram.gdextension -> libtdjson.so.X.Y.Z
  * thirdparty/td/CMakeLists.txt            -> project(TDLib VERSION X.Y.Z)
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def main() -> int:
    match = re.search(r'tdlib_version\s*=\s*"([^"]+)"', read("SConstruct"))
    if match is None:
        print("ERROR: tdlib_version not found in SConstruct")
        return 1
    version = match.group(1)
    print(f"Reference version (SConstruct): {version}")

    errors = []

    def check(name: str, pattern: str, text: str) -> None:
        found = set(re.findall(pattern, text))
        if not found:
            errors.append(f"{name}: version reference not found")
        elif found != {version}:
            for value in sorted(found):
                if value != version:
                    errors.append(f"{name}: found {value}, expected {version}")
        else:
            print(f"  OK {name}: {version}")

    check("tests/test_runner.gd", r'const TDLIB_VERSION\s*=\s*"([^"]+)"', read("tests/test_runner.gd"))
    check(".github/workflows/ci.yml", r"libtdjson\.(?:so\.)?(\d+(?:\.\d+)*)", read(".github/workflows/ci.yml"))
    for gdextension in (
        "addons/godot-tdlib/telegram.gdextension",
        "example/addons/godot-tdlib/telegram.gdextension",
        "tests/addons/godot-tdlib/telegram.gdextension",
    ):
        check(gdextension, r"libtdjson\.(?:so\.)?(\d+(?:\.\d+)*)", read(gdextension))
    check(
        "thirdparty/td/CMakeLists.txt",
        r"project\(TDLib VERSION ([0-9][0-9.]*)",
        read("thirdparty/td/CMakeLists.txt"),
    )

    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1
    print("All TDLib version references are consistent.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
