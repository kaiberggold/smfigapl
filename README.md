# Introduction
smfigapl (Small field game Planer) is a command line tool to generate a game plan. It is specially designed for the requirements of small field soccer tournaments:
- A small number of teams (e.g. 8 Teams)
- Each Club has 1-3 Teams
- Parallel games on several playing fields, typcially four fields.
- Game Plans are not complete, e.g. odd number off teams, Number of Teams > Number of Rounds +1, etc.
- Different types of playing fields (Two large goals or four small goals)
- Typically 6-8 Playing rounds.
- Often odd numbers e.g. 7 Teams, so teams have to pause
- Some side conditions (e.g. optimally no or a small number of games within the same club, an equal distribution over the different playing field, pauses distributed equally within the different teams, etc.)
- for a large amount of teams split in two subtournaments

smfigapl reads the basic configuration (team and club names, field types and names) and the side conditions from a configuration file, and generates a gameplan, which fullfills the side conditions. The output is either a simples text file (-t option), or a csv-Style file designed to be imported in a spreadsheet program. An example libreoffice sheet can be  found in the samples folder. The sheet then be formated in a way to obtain a nice printable gameplan.


# GamePlan
The generated gamplan can be imported with a spreadsheet programm, e.g. libreoffice. Here the sample sheet (See *samples/LANGUAGE/basic/tournament.ods*)). 

Teams colors are autoformated via a hidden color row. Gameplan is generic, unused rows are filtered via a generated filter row. (Note that the filter does not update automatically yet). 

![GamePlan](images/003_4-fields_7-teams.png?raw=true)

The generated statistic is shown here:
![GamePlan](images/003_4-fields_7-teams_statistic.png?raw=true)

# Basic Usage
```
Usage:
  ./build/Debug/bin/smfigapl [OPTION...]

  -h, --help               Print usage
  -v, --version            Print version
  -d, --debug              Debug
  -e, --debugoptions arg   Debug Options
  -w, --stopwatch          Enable Stopwatch
  -m, --maxsteps N         Maximum search steps
  -p, --progress           Print Progress
  -s, --statistic          Print Statistic (For text output)
  -t, --text               Simple Text Output
  -z, --stepsize N         Steps printing spacing
  -i, --input FILE         Input File
  -f, --format FILE        Format File
  -l, --language LANGUAGE  Language (EN,DE) (default: DE)
  -o, --output FILE        Output File
```

# cfg-Files
Teams, Fields, conditions etc. are defined in a config File, which useses the [Toml format](https://github.com/toml-lang/toml). Basic sample Files are located in the folder samples/basic (or samples/german/basic). The files hav then extension *.toml*. 
Output format file may be in seperate file (-f) option, to allow the usage of one format file for different cfg-Files. 

# Example cfg-File
```
# Number of Rounds to play (Range: 1-10)
rounds = 7

# Fields (Range: 1-10)
# Fields are ordered in the order of definition.
# For every field there are three fields:
# - type: This is used for the fieldtype conditions, and for the output file
# - subtype: Used to restrict certain teams to certain subtypes. Can be used two divide the tournamen in subtournaments, but is not restricted to that.
# - name: The name of the filed used in the Output File
   
[[fields]]
    type = "Small Goals"
    subtype="Left"
    name="1"
[[fields]]
    type = "Large Goals"
    subtype="Left"
    name="2"
[[fields]]
    type = "Small Goals"
    subtype="Right"
    name="3"
[[fields]]
    type = "Large Goals"
    subtype="Right"
    name="4"

# Clubs (Range: 1-16)
# Clubs are ordered in the order of definition.
# For every field there are two fields:
# - name: This is used for the same_club condition and for the output file
# - colors: Defines the Teams within a Club. Typically these are colors corresponing to the trikot color, but theres no restriction to colors

[[clubs]]
    name = "North"
    colors = [ 'Red', 'Green'] 
[[clubs]]
    name = "South"
    colors = [ 'Red', 'Blue'] 
[[clubs]]
    name = "West"
    colors = [ 'Red', 'Green'] 
[[clubs]]
    name = "East"
    colors = [ 'Red', 'Black'] 
    
# Conditions
# Side Conditions can be restricted by maximum / minimum Values. All conditions have to be fullfilled to obtain a valid gameplan 
# If not solution is found, try to widen the windows between min an max values 
# For some conditions, only one restricion is poassible
# The Pairing condition is always active (every pairing may only occur once, or once every iteration.

[conditions]
	# Pause games: Only max Value. Every team has no more than max rounds without a game.
    [conditions.pause_games]
        max = 1
    # Field types: Every Teams has within min/max Games on the specified fieldtype. "name" has to match on of the field types defined in the fields definitions.
    [[conditions.field_types]]
        name = "Small Goals"
        min = 3
        max = 4
    [[conditions.field_types]]
        name = "Large Goals"
        min = 3
        max = 4
    # Same Club: Only max Value. Every Team has up to max games with teams which belong to the same club.
    [conditions.same_club]
        max = 2
    # Only max. For every round there are only up to max fields, where is no game.
    [conditions.empty_fields]
        max = 1

	# Sub Type conditions are used to restrict certain teams to a subset of the available fields. 
	# The Restriction of  Subsets of Teams to a Subsets of fields may be used to obtain sub tournaments
	# The teams_deny list has to habe am even number of strings, club + color for each team. 
	#[[conditions.subtypes]]
  	#name = "Left" 
    #teams_deny = ['North', 'Red']
```

# Language support
Specifier are defined in differen Languages, so far english (EN) and german (DE). See samples/german for german examples. Languages are defined in the class *sm_lang*. Toml itself is case sensitive, but here specifiers are accepted in three types of capitalization, e.g. rounds, Rounds, ROUNDS.

# Build
smfigapl is written in c++17 and uses the cmake build system. It builds with gcc (10.2.0), clang (10.0.1), and MSVC (??). A ```  cmake . and make ```   should be sufficient to build the code. It requires some external libraries, which are included as submodules:
- [googletest](https://github.com/google/googletest) (for some basic automatic tests)
- [toml++](https://github.com/marzer/tomlplusplus) (for parsing the configuration files)
- [cxxopts](https://github.com/jarro2783/cxxopts) ( for parsing the commandline options)
- [stopwatch](https://github.com/CrikeeIP/Stopwatch) ( for runtime measurement)

# Test
Automatic regressions tests are performed via google test. The test use cfg-Files and check the output of the sm_search class. The condition checks use the results of the sm_stat class to check, if the defined conditons are fullfilled. They also check the number of searchsteps in comparison to a stored value for each test. The second set of tests check the input validation and the error messages.

# Configuration
There are some configration defines :
- SM_DBG: Activate debug code 
- SM_JUMP_FAST_LEVELDOWN: Activate Jump prediction when going to next search level (should be on, only for debugging)
- SM_FAST_JUMP: Activate Jump prediction (should be on, only for debugging)
- SM_MAX_FT_JUMP: jump prediction for fieldtypes  (should be on, only for debugging)
- SM_DEFAULT_LANGUAGE "DE" : The default Language
- SM_CATCH_ALL: catch all exceptions with a standard error text
- SM_STREAM_STDIN read input file from stdin (default off)

# Algorithm
- input file is read in *sm_file*, and and a representions of teams, fields, etc. based on numeric indices is calculated and stored in *sm_in*, as well as settings, mapping to the string based representation, etc. 
- input data shall be validated in sm_val, this is very basic so far.
- the algorithm is basically a treesearch done in *sm_search*. All possible combinations for a round are calculated in *sm_ts*. Here, the Teams, fields are transformed to a  normalized  gameplan, where the number of teams is equal to to number of fields, and the fields are sorted that fieldtypes are grouped. Additional virtual teams /fields /fieldtype are used to fill up the gameplan. Then all principle possible pertubations are calculated. Home-Away positions are ignore here. 
- Conditions are calculated differentially during the tree search, to do as less calculations as possbible during the treesearch. The Tables used for the counters are precalculated in sm_search and sm_cond. 
- The normalized order off the pertubations is used to calc jump tables, which are used to go fast forward in the treesearch if it is know from the current conditons the following steps won't match the conditions too.
- When all conditions are fullfilled, the search is stopped. The resulting gameplan is optimized to obtain a equl home/away distribution, and to avoid game repetitions in a row ( if repetitions are allowed).
- Finally gameplan is transformed back to a "named" gameplan with the original output mapping, statistics are calculated in *sm_stat*, an gameplan is printed in *sm_disp*.
- exceptions (invalid files, values out of range, not solvables conditions, etc.) are handled in *sm_except*.
- various classes needed in different context are defined in *sm_misc*

# Todo
- better input validation (error messaging, detect unsolvable conditions directly)
- more tests (input validations, special cases)
- better integration in spreadsheet tool.

# Contribute
Are welcome. For Bug reports please attach cfg-File. 

# License

smfigapl is licensed under the terms of the MIT license - see LICENSE.

# Contact
smfigapl@gmail.com
