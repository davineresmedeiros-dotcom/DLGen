# DLGen

**DLGen** is a Geometry Dash layout generator mod for [Geode](https://geode-sdk.org/).

DLGen's keyword settings cover:

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

The repository currently contains the Geode mod manifest, CMake setup, entry point, and keyword configuration/parser foundation. The next milestone is the in-game configuration UI and the actual Geometry Dash editor object-generation logic.

## Build

Configure with the Geode SDK path:

```sh
cmake -B build -DGEODE_SDK=/path/to/geode-sdk
cmake --build build
```

## License

MIT. See [LICENSE](LICENSE).
