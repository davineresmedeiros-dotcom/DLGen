# DLGen

**DLGen** is a Geometry Dash layout generator mod for [Geode](https://geode-sdk.org/).

- Geode mod ID: `davinxs.DLGen`
- Expected compiled mod file: `davinxs.DLGen.geode`
- This project builds a Geode mod package (`.geode`), **not an APK**.

DLGen keyword settings:

- **Difficulty:** easy, normal, hard, insane, demon
- **Length:** medium, long, XL (XL target: 300 seconds / five minutes)
- **Orbs:** active or inactive
- **Triggers:** enabled or disabled

Example keyword input:

```text
demon XL orbs ativo triggers nao
```

The parser also accepts forms such as `orbs=active` and `triggers=no`.

## Project status

The repository currently contains the Geode mod manifest, CMake setup, entry point, and keyword configuration/parser foundation. The next milestone is the in-game configuration UI and actual Geometry Dash editor object-generation logic.

## Build

Configure and build using the Geode SDK. The output must be the Geode package named `davinxs.DLGen.geode`.

```sh
cmake -B build -DGEODE_SDK=/path/to/geode-sdk
cmake --build build
```

## License

MIT. See [LICENSE](LICENSE).
