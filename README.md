# DLGen

**DLGen** is a Geometry Dash layout generator mod for [Geode](https://geode-sdk.org/).

DLGen will generate playable layout sections from keyword settings:

- **Difficulty:** easy, normal, hard, insane, demon
- **Length:** medium, long, XL (target: about five minutes)
- **Orbs:** active or inactive
- **Triggers:** enabled or disabled

## Development

DLGen targets Geode SDK and Geometry Dash. The first milestone is the keyword configuration and generator foundation; actual in-editor object placement will be implemented in subsequent steps.

## Build

Use the Geode SDK build environment and CMake:

```sh
cmake -B build -DGEODE_SDK=path/to/geode-sdk
cmake --build build
```

## License

MIT. See [LICENSE](LICENSE).
