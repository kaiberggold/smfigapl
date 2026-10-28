# Formats {#formats}

The domain model (smfigapl::TournamentSetup, smfigapl::layout::OutputLayout)
does not depend on a storage format. A format adapter turns it into text
and back; today there is the YAML adapter (smfigapl/formats/yaml.hpp).

## YAML

- smfigapl::formats::yaml::readSetup and writeSetup: the setup (rounds,
  fields, clubs, conditions); the key `format_version`.
- smfigapl::formats::yaml::readLayout and writeLayout: the output layout
  (the maps `information` and `format`); the key `layout_version`.

The setup and the layout can be in one text or in two (each reader
ignores the keys of the other). The keys are English, lower case, words
joined by `_`. Unknown keys and newer versions are warnings; syntax errors,
duplicate keys and values of the wrong type are errors with their line and
column. A name (of a field, a club, a team) may be written as a number
(`name: 1`), it is read as text. The writers quote texts that other YAML
readers could take for a number, a boolean, a date or null, and reading
what was written gives the same value.

### The setup

```yaml
format_version: 1            # optional
rounds: 7                    # required
iterations: 1                # how often a pairing may be played (with repeat_pairings)
repeat_pairings: false
home_away_balance: true
optimize_round_order: true
fields:                      # required, in the order of the plan
  - {name: '1', type: Small Goals, subtype: Left}
clubs:                       # required
  - name: North
    teams: [Red, Green]
conditions:                  # optional, every key optional
  max_pauses: 1
  max_same_club_games: 2
  max_empty_fields: 1
  field_types:
    - {type: Small Goals, min: 3, max: 4}   # min or max may be left out
  subtype_deny:
    - subtype: Left
      teams:
        - {club: North, team: Red}
```

The keys are the members of smfigapl::TournamentSetup in lower case with
`_`; the paths of the problems use the member names (e.g.
`conditions.maxPauses`, `fields[2].type`).

### The layout

The layout of the CSV output for a spreadsheet (see
*samples/english/basic/format.yaml*): `information` holds free texts
(`general`) and dates and times (`date_time`; `YYYY-MM-DD`, `HH:MM:SS` or
`HH:MM`, or both joined by `T` or a space), `format` the positions of the
parts of the plan in the sheet.

### Schemas

*docs/schema/setup.schema.json* and *docs/schema/layout.schema.json*
(JSON Schema) describe the keys of the two texts. Editors with a YAML
language server use them for completion and checks; the samples are
checked against them. The reader of the library does not need them: it
checks the texts itself.

## Versions of the formats

A format version changes when a key changes its meaning or a required key
is added. A reader reads older versions; a newer version is read as far
as possible with a warning (ProblemCode::unsupportedFormatVersion). The
format versions are independent of the library version.

## Another format

A new adapter provides the same four functions and passes the format
independent round trip tests (*test/formats*, a typed test suite: add the
adapter to the list of types).
