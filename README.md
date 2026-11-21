# DLGen

**DLGen** is a Geometry Dash layout generator mod for [Geode](https://geode-sdk.org/).

- Geode mod ID: `davinxs.DLGen`
- Expected compiled mod file: `davinxs.DLGen.geode`
- This project builds a Geode mod package (`.geode`), **not an APK**.

## Choose layout options

Open DLGen's in-game Geode settings and choose each value from its available options. These are selections, not keyword text inputs:

- **Difficulty:** Easy, Normal, Hard, Insane, Demon
- **Time:** Medium, Long, XL (XL target: 300 seconds / five minutes)
- **Orbs:** Active, Inactive
- **Triggers:** Yes, No

## Project status

The repository contains the Geode manifest, CMake setup, entry point, and selectable in-game configuration options. The next milestone is connecting these choices to actual Geometry Dash editor object-generation logic.

## Build

Configure and build using the Geode SDK. The output must be the Geode package named `davinxs.DLGen.geode`.

```sh
cmake -B build -DGEODE_SDK=/path/to/geode-sdk
cmake --build build
```

## License

MIT. See [LICENSE](LICENSE).
