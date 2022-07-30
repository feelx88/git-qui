![git-qui](https://raw.githubusercontent.com/feelx88/git-qui/master/de.feelx88.git-qui.svg?sanitize=true "git-qui")

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

A Mac OS installer version is available here: https://repo.git-qui.feelx88.de/mac/installer-mac-x86_64.dmg

#### Updating/Uninstalling

The installer ships with a binary called `maintenancetool` which can be used to update or uninstall the program.

#### Without installer

A version without the installer is available under [releases](https://github.com/feelx88/git-qui/releases) too.

### Building from source

This is actually really simple: clone the repository, open the `.pro` file in QtCreator and build.
You can also build everything via the command line: `qmake && make`

# Acknowledgments

The Logo is based on the Git Logo by [Jason Long](https://twitter.com/jasonlong) (licensed under the [Creative Commons Attribution 3.0 Unported License](https://creativecommons.org/licenses/by/3.0/)).

Icons packaged with the program are part of [breeze icon set](https://github.com/KDE/breeze-icons).

