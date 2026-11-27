"""Compile and run the real C++ localization/catalog headers, without game/DMA.

Run in a VS Developer Command Prompt: python tools/localization/verify.py
--source-archive permits the same regression against a baseline git archive.
"""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile

import generate_names

ROOT = generate_names.ROOT


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-archive", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    folder = args.output or Path(tempfile.mkdtemp(prefix="veox-localization-"))
    folder.mkdir(parents=True, exist_ok=True)
    sources = ["VeoxDMA/Misc/Localization.h", "VeoxDMA/Misc/LocalizationNames.h",
               "VeoxDMA/Config/LootFilterData.h"]
    archive = zipfile.ZipFile(args.source_archive) if args.source_archive else None
    try:
        for source in sources:
            if archive and source not in archive.namelist():
                continue
            dest = folder / source
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(archive.read(source) if archive else (ROOT / source).read_bytes())
    finally:
        if archive:
            archive.close()
    (folder / "VeoxDMA/Misc/ConfigInstance.h").write_text(
        "#pragma once\ninline struct { struct { bool ChineseLanguage = false; } Overlay; } ConfigInstance;\n",
        encoding="utf-8")
    catalog = (folder / sources[2]).read_text(encoding="utf-8-sig")
    sdk = folder / "VeoxDMA/SDK/WorldEntities.h"
    sdk.parent.mkdir(parents=True, exist_ok=True)
    sdk.write_text("#pragma once\nnamespace RobRunWorld { enum class Kind { "
                   + ",".join(sorted(set(re.findall(r"RobRunWorld::Kind::(\w+)", catalog))))
                   + " }; }\n", encoding="utf-8")
    values = generate_names.translations()
    expected = "#pragma once\ninline const std::unordered_map<std::string, std::string> kExpectedNames = {\n"
    for en, zh in sorted(values.items()):
        expected += "    {" + json.dumps(en, ensure_ascii=False) + ", " + json.dumps(zh, ensure_ascii=False) + "},\n"
    (folder / "expected_names.h").write_text(expected + "};\n", encoding="utf-8")
    shutil.copy2(Path(__file__).with_name("regression.cpp"), folder / "regression.cpp")
    command = ["cl", "/nologo", "/EHsc", "/std:c++20", "/utf-8", "/Fe:regression.exe", "regression.cpp"]
    if "TranslateItemName" in (folder / sources[0]).read_text(encoding="utf-8-sig"):
        command.insert(1, "/DHAS_ITEM_NAMES")
    built = subprocess.run(command, cwd=folder, capture_output=True)
    (folder / "compile.log").write_bytes(built.stdout + built.stderr)
    if built.returncode:
        print(built.stdout.decode(errors="replace"))
        raise SystemExit(built.returncode)
    result = subprocess.run([str(folder / "regression.exe")], cwd=folder, capture_output=True)
    (folder / "result.log").write_bytes(result.stdout + result.stderr)
    print(result.stdout.decode("utf-8", errors="replace"), end="")
    raise SystemExit(result.returncode)


if __name__ == "__main__":
    main()
