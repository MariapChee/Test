# Crynos Executor v2.0

Advanced Roblox Script Executor built with C++ and Dear ImGui.

## Features

- **Multi-Tab Script Editor** with line numbers, word wrap, auto-indent, and tab renaming
- **Dual Console** (Crynos + Roblox) with filtering (All/Info/Warn/Error/Roblox), text search, timestamps, auto-scroll
- **Script Hub** with 4 search APIs:
  - **ScriptBlox** - The largest Roblox script database
  - **Rscripts** - Community-driven script collection
  - **ScriptSearch** - Fast script search engine
  - **RawScripts** - Raw script repository
- **AI Chat Assistant** - GPT-4o-mini powered Lua/Luau script generator with code insertion
- **Saved Scripts Manager** - Save, export (.lua), search, and organize scripts locally
- **Advanced Settings**:
  - 10 accent colors (Cyan, Blue, Purple, Pink, Red, Orange, Yellow, Green, Teal, Indigo)
  - 7 background themes (Dark, Darker, Navy, Slate, Charcoal, Midnight, Obsidian)
  - Transparency slider
  - 22 languages
  - Font size, FPS unlock, auto-save, and more
- **FPS Counter** with color-coded performance display
- **Notification System** with animated popups
- **Keyboard Shortcuts**: Ctrl+Enter (Execute), Ctrl+S (Save), Ctrl+N (New Tab), Ctrl+1-5 (Switch Panels)
- **Roblox Integration** via global JS bridge or HTTP API

## Build

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install -y cmake build-essential libglfw3-dev libgl1-mesa-dev libcurl4-openssl-dev pkg-config

# Arch Linux
sudo pacman -S cmake glfw-x11 curl

# macOS
brew install cmake glfw curl
```

### Compile

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run

```bash
./build/CrynosExecutor
```

## Project Structure

```
src/
  main.cpp          - Entry point
  app.cpp           - Main application loop and window management
  editor.cpp        - Multi-tab script editor
  console.cpp       - Dual console with filtering
  script_hub.cpp    - Script Hub with 4 API integrations
  saved_scripts.cpp - Local script storage and management
  settings.cpp      - Settings panel and persistence
  ai_chat.cpp       - AI-powered Lua script generator
  http_client.cpp   - Async HTTP client (libcurl)
  theme.cpp         - Theme engine with accent/background colors
  utils.cpp         - Utility functions

include/
  imgui/            - Dear ImGui (vendored)
  json/             - nlohmann/json (vendored)
  stb/              - stb_image (vendored)
  *.h               - Application headers
```

## Tech Stack

- **C++17**
- **Dear ImGui** - Immediate mode GUI
- **GLFW** - Window management
- **OpenGL 3.3** - Rendering
- **libcurl** - HTTP requests
- **nlohmann/json** - JSON parsing
- **stb_image** - Image loading
