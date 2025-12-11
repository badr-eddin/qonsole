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

## Usage: Local PTY terminal

```cpp
#include "qonsole/qonsole.hpp"
#include <QApplication>

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <stdio.h>
#include <pty.h>
#include <termios.h>
#include <sys/ioctl.h>


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qonsole::QonsoleWidget *qns = new qonsole::QonsoleWidget(nullptr);

    // Create a shell attached to a PTY and get the master fd
    int fd = create_process_pty("/bin/bash");

    if (fd < 0) {
        qDebug() << "error creating process";
        return 1;
    }

    qns->set_reader(new qonsole::QonsoleReader(fd));

    // customization
    qns->set_font(QFont("Fira Code", 13));
    qns->set_bold(false);
    qns->set_cursor_style(qonsole::BLOCK);
    
    qns->show();
    return app.exec();
}
```
