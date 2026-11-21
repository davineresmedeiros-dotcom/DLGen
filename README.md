# DLGen

**DLGen** is a Geometry Dash layout generator mod for [Geode](https://geode-sdk.org/).

- Geode mod ID: `davinxs.DLGen`
- Expected compiled mod file: `davinxs.DLGen.geode`
- This project builds a Geode mod package (`.geode`), **not an APK**.

## In-editor interface

DLGen adds a **DLGen** shortcut button to the level editor's playback menu. Press it to open the mod's Geode settings, where each layout option is selected from a list.

- **Difficulty:** Easy, Normal, Hard, Insane, Demon
- **Time:** Medium, Long, XL (XL target: 300 seconds / five minutes)
- **Orbs:** Active, Inactive
- **Triggers:** Yes, No

These are selectable options, not keyword text inputs.

## Project status

The initial editor shortcut and selectable settings are committed. The next milestone is a dedicated generator panel and connecting the selected options to actual Geometry Dash editor object-generation logic.

## Build

Configure and build using the Geode SDK. The output must be the Geode package named `davinxs.DLGen.geode`.

```sh
cmake -B build -DGEODE_SDK=/path/to/geode-sdk
cmake --build build
```

## License

MIT. See [LICENSE](LICENSE).
