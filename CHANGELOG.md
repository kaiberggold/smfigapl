# Changelog

The releases of smfigapl (see *docs/guides/versioning.md*). 0.5.0 is the
first release with the library API; v0.0.1 (2021) was the command line
tool alone.
Complete rework, c++20, static code checks, solid principles, cleanup

## [0.7.0] - 2026-11-07

### Library
- `checkPlan(setup, plan)` checks a given game plan, e.g. one changed by
  hand, against its setup: the conditions it breaks are problems with the
  new codes 400 to 411 (errors when the plan does not fit the setup; a game
  with one team is an open game).
- Fixed: with fields whose types or subtypes alternate (not grouped by
  type in the setup), the games of a round were put on the wrong fields,
  so field type limits and denied subtypes could be broken in a plan
  reported as solved. The pairings and pauses were right; now every game
  is on the field the search chose for it. Plans of such setups change.

## [0.6.0] - 2026-11-03

### Library
- Builds with a libc++ older than version 20, e.g. the Android NDK r27
  that React Native uses: `std::stop_token` is still experimental there,
  so CMake adds `-fexperimental-library` for the library and the targets
  that link it.

## [0.5.0] - 2026-10-28

The first version with a public API.

### Library
- Public API in *include/smfigapl*: the setup (`TournamentSetup`), the
  result (`SolveResult`), the statistic, the output layout, problems with
  codes, values and paths (`Result`, `Problem`), the version.
- Planner (*planner.hpp*): `validate`, `solve` (step and time limit,
  cancellation with a `std::stop_token`, progress, a `LogSink`) and
  `computeStatistic`; no exceptions, several searches at once.
- YAML adapter (*formats/yaml.hpp*) for the setup and the layout, with
  `format_version` / `layout_version`, warnings for unknown keys, line and
  column of every problem; JSON schemas of both texts in *docs/schema*.
- Output writers (*report/*): text and CSV, with the texts from the caller.
- CMake package (`find_package(smfigapl)`, target `smfigapl::smfigapl`)
  and FetchContent; generated documentation (Doxygen).

### Command line tool
- The configuration and format files are YAML (*.yaml*) instead of TOML.
  The keys are English and lower case only, and some are renamed to the
  names of the setup: `colors` → `teams`, `optimize_rounds` →
  `optimize_round_order`, `[conditions.pause_games] max` → `max_pauses`,
  `same_club` → `max_same_club_games`, `empty_fields` →
  `max_empty_fields`, `field_types.name` → `type`, `subtypes` with
  `teams_deny` pairs → `subtype_deny` with `teams` of `club` and `team`.
  The samples show the new files.
- Uses the public API; its texts and messages are its own.
- Prints every problem of the input (the exit code is the first error).
- Warnings of the configuration ("Warning number ...").

### Fixed
- The statistic counted a pause as a game against the own club.
- With separate tournaments the CSV showed the pausing teams of the first
  sub tournament in the others.
- The German sample's condition `Leere_Felder` (empty fields) was ignored.
- Values of a wrong type in the configuration were ignored, wrapped around
  or crashed; they are errors now.
- The last of 16 clubs could have only one team.
- Field type conditions of sub tournaments with different field types,
  the field type labels of the statistic, the padding of names with
  umlauts in the text output.
