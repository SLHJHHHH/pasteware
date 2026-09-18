# pasteware

An internal client modification for Counter-Strike 1.6 (GoldSrc engine) built on top of Dear ImGui and OpenGL.

## Features

### Combat
- Legitbot: Customizable FOV, smoothing, target hitgroups, and recoil compensation.
- Ragebot: Automatic fire, minimum damage penetration calculation, hitchance, and resolver.
- Triggerbot: Configurable delay, trace accuracy, and scoped-only checks.

### Anti-Aim & Exploits
- Desync, static pitch/yaw angle adjustments, and movement direction modifiers.
- Fake lag generator with configurable choke limits and peek triggers.
- Doubletap and tick-shifting logic.

### Visuals
- Player ESP: 2D bounding boxes, player health bars, skeletons, names, and equipped weapon text.
- Chams: Model recoloring, flat/textured styles, backtrack records, and wireframe modes.
- Local effects: Bullet tracers, third-person camera mode, and custom crosshair rendering.

### Movement
- Automated bunnyhop, ground strafe speed gain, fast run, and bug assists (jumpbug, edgebug, wallbug).

### Interface
- Primordial-style layout with dynamic tab navigation and vector icon atlases.
- Custom animated widgets, inline number editing on sliders, and full keybind assignment support.
- Color theme customization with clipboard copy/paste support.
- Integrated configuration manager.

## Building from Source

### Prerequisites
- Windows 10 / 11
- Microsoft Visual Studio 2022 (or newer) with Desktop Development with C++
- MSVC Toolset (v143 or v145)
- Windows SDK (10.0 or 11.0)

### Compilation Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/SLHJHHHH/pasteware.git
   ```
2. Open `pasteware.sln` in Visual Studio.
3. Set the build configuration to **Release** and platform to **x86 (Win32)**.
4. Build the solution (`Ctrl + Shift + B`) or compile via Developer Command Prompt / MSBuild:
   ```bash
   msbuild pasteware.vcxproj /p:Configuration=Release /p:Platform=Win32
   ```
5. The compiled dynamic link library will be generated at `Release/pasteware.dll`.

## Project Structure
- `src/features/` - Combat, movement, visual, and miscellaneous feature modules.
- `src/overlay/` - ImGui rendering backend, Primordial UI layout, and UI components.
- `src/world/` - Entity, weapon, and player tracking logic.
- `src/tools/` - Math utilities, memory manipulation, and engine wrappers.
- `includes/` - Third-party libraries (ImGui, MinHook, SOIL, SimpleIni, ValveSDK).

## Disclaimer
This project is developed solely for educational and research purposes.
