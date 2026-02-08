# git-qui

Qt6-based UI replacement for `git-gui` and `gitk`, designed for a modern and flexible user experience.

## Project Overview

*   **Core Technology:** C++20, Qt 6.2+
*   **Main Libraries:**
    *   Qt (Core, Gui, Widgets, Svg, Xml)
    *   [Qt Advanced Docking System](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) (integrated as a submodule, providing the dockable UI)
    *   QtAwesome (present as a submodule, but currently unused in the code)
*   **Architecture:**
    *   `src/main.cpp`: Entry point, sets up `QApplication` and the `Core` engine.
    *   `src/core.hpp/cpp`: Manages the application state and projects.
    *   `src/gitinterface.hpp/cpp`: The heart of the application, wrapping git commands via `QProcess` and providing an asynchronous API using `QFuture`.
    *   `src/components/`: Modular UI components (dockable widgets) for various Git functionalities (Log, Diff, Branch list, etc.).
    *   **Icons:** Icons are primarily loaded from the system theme or fall back to custom SVGs in `deploy/icons/` (managed via `resources.qrc`).

## Building and Running

### Prerequisites

*   Qt 6.2 or higher
*   CMake 3.20 or higher
*   A C++20 compatible compiler
*   Git (available in PATH)

### Build Commands

```bash
# Ensure submodules are initialized
git submodule update --init --recursive

mkdir build
cd build
cmake ..
make
```

### Translations

To update or release translations (e.g., `git-qui_de.ts`):

```bash
make translations
```

## Development Conventions

*   **Coding Style:** Follows standard Qt coding conventions. Use `.clang-format` for automated formatting.
*   **Git Interaction:** Always use `GitInterface` for executing git commands. It handles threading and error logging.
*   **UI Design:** Uses `.ui` files for layout, managed by `Qt Designer`. The application uses a dock-based UI via `Qt Advanced Docking System`.
*   **Icons:** When adding new actions, prefer system theme icons with a fallback to `deploy/icons/`.
*   **Formatting:** For formatting QML files (if any), use `/usr/lib/qt6/bin/qmlformat`. For C++ files, follow the existing style and use the provided `.clang-format`.

## Key Files

*   `CMakeLists.txt`: Main build configuration.
*   `src/gitinterface.cpp`: Implementation of git command wrappers.
*   `resources.qrc`: Qt resource file for icons and assets.
*   `deploy/icons/`: Directory containing fallback SVG icons.
*   `de.feelx88.git-qui.yml`: Flatpak manifest for distribution.
*   `.gitmodules`: Defines submodules (`Qt-Advanced-Docking-System`, `QtAwesome`).
