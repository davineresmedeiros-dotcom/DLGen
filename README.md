# DLGen

**DLGen** is a Geometry Dash layout generator mod for [Geode](https://geode-sdk.org/).

- Mod ID: `davinxs.DLGen`
- Intended game: Geometry Dash **2.2081**
- Intended loader: Geode **5.11.2**
- Expected package: `davinxs.DLGen.geode` (not an APK)

## In-editor controls

The editor playback menu has two buttons:

- **DLGen** opens the mod settings with selectable options.
- **Generate** creates a safe starter layout in the open level.

Options:
- **Difficulty:** Easy, Normal, Hard, Insane, Demon
- **Time:** Medium (60 s), Long (120 s), XL (300 s / five minutes)
- **Orbs:** Active, Inactive
- **Triggers:** Yes, No

## Playability-first generation

The current starter generator lays a continuous solid floor and, when Orbs is Active, places optional yellow orbs above it. It deliberately does not place spikes or forced-jump hazards, so this first generated route avoids impossible obstacle patterns.

Difficulty currently changes the spacing of optional orbs. The Triggers selection is present in settings but is not yet connected to trigger placement. Advanced obstacle patterns and gameplay verification are still in development.

## Build

Configure and build with a compatible Geode SDK for the intended game version. The output must be the Geode package `davinxs.DLGen.geode`.

```sh
cmake -B build -DGEODE_SDK=/path/to/geode-sdk
cmake --build build
```

## License

MIT. See [LICENSE](LICENSE).
