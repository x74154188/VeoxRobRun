# Item and vehicle localization

English remains the default (`Overlay.ChineseLanguage = false`). The language
switch is in the menu's Settings tab. Chinese names apply only at display time:
catalog rows/popups, world ESP, radar/map labels and equipped weapon text.
Registry IDs, sprite keys, filters, prices and saved custom names stay intact.

`names.zh-CN.tsv` contains exact English/Chinese display-name pairs and SDK
fallback names. Model identifiers and abbreviations remain recognizable.
`generate_names.py` adds exact registry ID aliases from `LootFilterData.h` and
generates `VeoxDMA/Misc/LocalizationNames.h`. It rejects missing names or
ambiguous aliases so an upstream catalog update cannot silently lose coverage.

After an upstream catalog update, add the new names to the TSV and run:

```powershell
python tools/localization/generate_names.py
python tools/localization/generate_names.py --check
```

In a Visual Studio Developer Command Prompt with Python and C++20 available:

```bat
python tools/localization/verify.py
MSBuild VeoxDMA.sln /m /p:Configuration=Release /p:Platform=x64 /v:minimal
```

The regression compiles the real localization and catalog headers, stubbing only
configuration/SDK dependencies. It checks every catalog name/ID, vehicle paint
alias, English round trip, Chinese/English search, unknown and custom labels,
and the menu/HUD format values and ImGui IDs. It does not require a game process.
For visual testing in game, switch Chinese on, inspect all catalog categories,
check ESP and full-map names with Loot Filter both on and off, then switch back
to English. Player nicknames and custom labels should remain as entered.

## Custom ESP font

Open **Settings → ESP Font → Choose Font File...** to browse Windows font files
(`.ttf`, `.otf`, `.ttc`). The font applies on the next frame to ESP text; menu
and status fonts remain unchanged. Existing text-size controls continue to work.
The preview shows the selected font at three sizes. Missing Chinese glyphs are
merged from an available system Chinese font. TTC files use their first face.

The UTF-8 path is saved as `Overlay.EspFontPath` in the current configuration.
An empty path restores the default. Canceling the file dialog changes nothing;
missing or invalid files display an error status and use the default font.
Use **Reset ESP Font** to remove the path.

In a VS Developer Command Prompt, run the real ImGui/font-atlas regression:

```bat
python tools/localization/verify_esp_font.py
```

It exercises Unicode path persistence, TTF/TTC rasterization, Chinese fallback,
ESP-only scoping, reset, corrupt/missing files and deferred atlas changes. The
picker's OS modal call is intercepted to test selection/cancellation without
opening a dialog on the desktop; the production picker uses `GetOpenFileNameW`.
