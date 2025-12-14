# Qonsole

A Qt-based terminal emulator widget using libtsm for terminal state management.

## Features

- Terminal rendering with libtsm
- Cross-platform support (Linux, macOS, Windows)
- Customizable color palettes (defaults to [Dracula theme](https://draculatheme.com))
- Multiple cursor styles (block, underline, I-beam)
- Configurable fonts
- Basic keyboard input handling (arrow keys, function keys, Ctrl combinations)
- Threaded input reader for file descriptors/handles/sockets
- Bold and underline text attributes

## Known Issues

- **Remote terminal resizing doesn't work** - window size changes only apply to local PTY connections

## Tested
- [x] Linux
- [] Windows
- [] Mac
