# smfigapl {#mainpage}

A library that searches game plans for small field tournaments: teams of
several clubs play on fields of different types, round by round, with
conditions such as how often a team may pause, how many games it plays
against teams of its own club, or how many games on each field type.

The command line tool of the repository uses this library; a mobile app
(React Native) will use it through a native module.

## The API in short

| Header | Content |
|---|---|
| smfigapl/setup.hpp | smfigapl::TournamentSetup: rounds, fields, clubs and teams, conditions |
| smfigapl/planner.hpp | smfigapl::validate, smfigapl::solve with smfigapl::SolveOptions, smfigapl::computeStatistic |
| smfigapl/plan.hpp | smfigapl::SolveResult: status, reasons, the game plan by name |
| smfigapl/statistic.hpp | smfigapl::Statistic: the numbers per team |
| smfigapl/result.hpp | smfigapl::Result, smfigapl::Problem, smfigapl::ProblemCode |
| smfigapl/formats/yaml.hpp | the YAML adapter: setup and layout as text |
| smfigapl/layout.hpp | smfigapl::layout::OutputLayout: the layout of the CSV output |
| smfigapl/report/text.hpp, report/csv.hpp | the output writers |
| smfigapl/log.hpp | smfigapl::LogSink for the debug messages |
| smfigapl/version.hpp | the version of the library |

The library has no texts: problems are codes with values, the output
writers get their labels (smfigapl::report::Labels) from the caller.

## Guides

- @ref getting_started "Getting started": add the library to a project, the first game plan
- @ref formats "Formats": the YAML adapter, the schemas, versions of the formats, other formats
- @ref versioning "Versioning": what a new version may change
- @ref ios_sources "Sources for iOS": how the app gets the sources
