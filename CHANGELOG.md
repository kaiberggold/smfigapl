# Changelog

The releases of smfigapl (see *docs/guides/versioning.md*). 0.5.0 is the
first release with the library API; v0.0.1 (2021) was the command line
tool alone.
Complete rework, c++20, static code checks, solid principles, cleanup

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
