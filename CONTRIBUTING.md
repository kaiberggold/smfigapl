# Contributing

Thank you for your interest in smfigapl.

## Issues

Bug reports, questions and ideas are welcome as
[issues](https://github.com/kaiberggold/smfigapl/issues). For a wrong or
missing game plan please attach the configuration file and the command
line (or the setup in code) and say which version you use
(`smfigapl -v`, or `smfigapl::VERSION_STRING`).

## Pull requests

This repository contains the releases: one commit per release. The
development happens in a separate repository, so a pull request is not
merged here. It is welcome all the same: the maintainer applies it in the
development repository, with you as co-author, and it comes out with the
next release (the pull request is closed with a note then).

Before you open one, please
- build with `-DSM_WARNINGS_AS_ERRORS=ON` and run the tests (`ctest`);
- format the code with astyle (`.astylerc`, see the README);
- keep the change small and describe it in the pull request; a line for
  *CHANGELOG.md* helps.

By contributing you agree that your contribution is licensed under the MIT
license of the project (LICENSE).
