# Sources for iOS {#ios_sources}

*Not decided yet: this is decided together with the app; verify with the
React Native version the app uses.*

On Android the app's native module is built with CMake (Gradle calls it),
so it adds the library with FetchContent like any CMake project
(@ref getting_started). iOS builds with CocoaPods/Xcode, not CMake, so the
podspec of the app's module needs the sources of a tag.

## What the sources are

- the public headers *include/smfigapl/**.hpp* and the generated
  *smfigapl/version.hpp* (CMake writes it from *include/smfigapl/version.hpp.in*);
- the sources and internal headers of the layers the app uses:
  *src/types*, *src/model*, *src/solver*, *src/api* and, if the app reads
  or writes YAML or CSV, *src/formats* and *src/report*;
- for *src/formats*: the sources of rapidyaml (the tag in *extern/CMakeLists.txt*:
  *src/c4/yml/\*.cpp* and *ext/c4core.src/c4/\*.cpp*, with *src* and
  *ext/c4core.src* as include paths and `RYML_WITH_TAB_TOKENS` defined);
- C++20, no other dependencies; no global state (several searches at once).

## Options

1. **Podspec with the git tag** (`s.source = { :git => ..., :tag => 'v0.5.0' }`)
   and `source_files` / `header_mappings_dir` per layer; a
   `prepare_command` writes *version.hpp* (a short script, or
   `cmake -P` with the version from *CMakeLists.txt*). Simple, but the
   podspec has to know the layout of *src/*.
2. **Release archive**: CI packs the sources of a tag with the generated
   *version.hpp* and a source list (an asset of the GitHub release); the
   podspec uses the archive. The layout stays inside this repository.
3. **XCFramework**: CI builds the library for device and simulator with
   CMake (the iOS jobs already compile it) and attaches a binary
   framework. Fastest app builds, but one binary per Xcode/C++ standard
   library version.

Recommendation: start with option 2 when the app needs it; the iOS CI
jobs (iOS 16, arm64, device and simulator) keep the sources compiling
for the app's targets.
