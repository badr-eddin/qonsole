#include "qonsole/qonsole.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <QApplication>

#include <stdio.h>
#include <pty.h>
#include <termios.h>
#include <sys/ioctl.h>

// Create a child process attached to a new PTY. Returns the master fd on
// success, or -1 on error. The child will exec the given shell (or /bin/sh
// if nullptr).
static int create_process_pty(const char* shell) {
    int master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd < 0) {
        return -1;
    }

    if (grantpt(master_fd) < 0 || unlockpt(master_fd) < 0) {
        close(master_fd);
        return -1;
    }

    char *slave_name = ptsname(master_fd);
    if (!slave_name) {
        close(master_fd);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(master_fd);
        return -1;
    }

    if (pid == 0) {
        // Child
        if (setsid() < 0) _exit(1);

        int slave_fd = open(slave_name, O_RDWR);
        if (slave_fd < 0) _exit(1);

        // Make the slave the controlling terminal
        if (ioctl(slave_fd, TIOCSCTTY, 0) < 0) {
            // not fatal on all platforms, continue
        }

        // Duplicate slave to stdin/stdout/stderr
        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);

        if (slave_fd > STDERR_FILENO) close(slave_fd);

        // Exec the shell
        const char* sh = shell ? shell : "/bin/sh";
        execlp(sh, sh, (char*)NULL);

        // If exec fails
        _exit(127);
    }

    // Parent: return master fd for use by the UI/reader
    return master_fd;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qonsole::QonsoleWidget *qns = new qonsole::QonsoleWidget(nullptr);

    qDebug() << "creating process ...";

    // Create a shell attached to a PTY and get the master fd
    int fd = create_process_pty("/bin/bash");

    if (fd < 0) {
        qDebug() << "error creating process";
        return 1;
    }

    qDebug() << "process created, sourcing ...";
    qns->set_reader(new qonsole::QonsoleReader(fd));

    qDebug() << "sourced process successfully, showing ...";

    // customization
    qns->set_font(QFont("Fira Code", 13));
    qns->set_bold(false);
    qns->set_cursor_style(qonsole::BLOCK);

    qns->widget_fit_vt_size();
    qns->set_vt_size(80, 24);

    // qns->resize(1000, 800);
    // qns->vt_fit_widget_size();
    
    qns->show();

    qDebug() << "no error";
    
    return app.exec();
}
