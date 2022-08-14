![git-qui](https://raw.githubusercontent.com/feelx88/git-qui/master/de.feelx88.git-qui.svg?sanitize=true "git-qui")

![CircleCI](https://img.shields.io/circleci/build/github/feelx88/git-qui)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/feelx88/git-qui?include_prereleases)

# git qui
qt5 ui replacement for git-gui.

## Installation

### Linux

The Linux version is available via flatpak - curently inside a self-hosted repository.

You can install it by opening this flatpakref file:

https://flatpak.feelx88.de/de.feelx88.git-qui.flatpakref

Alternatively, this can be run in the terminal:

`flatpak install https://flatpak.feelx88.de/de.feelx88.git-qui.flatpakref`

### Mac OS

An *untested* Mac OS version is automatically built for each release and can be found under [releases](https://github.com/feelx88/git-qui/releases).

### Building from source

This is actually really simple: clone the repository, open the `.pro` file in QtCreator and build.
You can also build everything via the command line: `qmake && make`

# Acknowledgments

The Logo is based on the Git Logo by [Jason Long](https://twitter.com/jasonlong) (licensed under the [Creative Commons Attribution 3.0 Unported License](https://creativecommons.org/licenses/by/3.0/)).

Icons packaged with the program are part of [breeze icon set](https://github.com/KDE/breeze-icons).

The application itself is heavily inspired by the git gui tools ([git gui](https://www.git-scm.com/docs/git-gui) and [gitk](https://www.git-scm.com/docs/gitk)) and uses some of the same ui concepts.
