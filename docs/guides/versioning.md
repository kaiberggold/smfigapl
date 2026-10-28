# Versioning {#versioning}

The library follows [Semantic Versioning](https://semver.org):
MAJOR.MINOR.PATCH, see smfigapl::version() and smfigapl/version.hpp.

The public API is everything in *include/smfigapl/*: the types, functions
and their documented behavior. Not part of it: the internal libraries and
headers under *src/*, the command line tool, the exact steps of a search.

| From 1.0 | Allowed |
|---|---|
| PATCH | fixes that keep the API; a fix may change a game plan only if the old one broke a condition |
| MINOR | new types, functions, members with defaults, problem codes, enum values at the end |
| MAJOR | everything else: removed or changed functions and members, changed meaning |

Stable in every version: the numbers of the problem codes (smfigapl::ProblemCode),
which are also the exit codes of the command line tool. The same setup
with the same version gives the same game plan.

Before 1.0 (now): a new MINOR version may change the API; the changes
are in *CHANGELOG.md*. The CMake package accepts only the same MINOR
version (`find_package(smfigapl 0.5)` takes 0.5.x). The maintainer
decides when 1.0 comes.

## Releases

Every release is one commit with the tag `vX.Y.Z` in this repository and a
GitHub Release with the notes from *CHANGELOG.md*. Use a release tag with
FetchContent (`GIT_TAG v0.5.0`). The development between releases happens
elsewhere; `VERSION_STRING` of a development build has a prerelease part
(e.g. `0.6.0-devel.3`, see smfigapl::VERSION_PRERELEASE), a release has none.
