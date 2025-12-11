#pragma once

/**
 * @file qonsole.hpp
 * @brief Qt-based terminal emulator widget using libtsm
 * 
 * @author badr-eddin
 * @see https://github.com/badr-eddin
 * 
 * @details
 * Qonsole is a Qt-based terminal emulator widget that leverages libtsm (Terminal State Machine)
 * for terminal emulation. It provides cross-platform support for Linux, macOS, and Windows,
 * with customizable colors, fonts, and cursor styles.
 * 
 * Features:
 * - Cross-platform terminal emulation (Linux, macOS, Windows)
 * - Configurable color palettes
 * - Multiple cursor styles (BLOCK, UNDERLINE, IBEAM, NONE)
 * - Font customization with bold and underline support
 * - Terminal size management with automatic PTY notifications (local only)
 * - Thread-safe data reading from file descriptors, pipes, or sockets
 * - Text selection support (framework ready)
 * 
 * @namespace qonsole
 * 
 * Key Components:
 * - q_selection: Represents text selection boundaries
 * - q_palette: Color palette definition for terminal colors
 * - q_cursor_pos: Cursor position tracking
 * - q_cursor_style: Cursor appearance enumeration
 * - QonsoleReader: Threaded reader for data input from various sources
 * - QonsoleWidget: Main terminal emulator widget
 * 
 * Platform-specific Features:
 * - Unix-like systems: Uses file descriptors and ioctl for PTY control
 * - Windows: Supports both file handles (pipes) and socket handles
 */


#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif // Bytes

#ifndef EMPTY_CELL_REPLACEMENT
#define EMPTY_CELL_REPLACEMENT " "
#endif // Bytes

// platform specific includes
#if defined(__linux__) || defined(__APPLE__)
    #include <unistd.h>     // read, write
    #include <sys/ioctl.h>  // ioctl, winsize
#elif defined(_WIN32)
    #include <windows.h>    // HANDLE
    #include <winsock2.h>   // SOCKET
#endif

#include <QPainter>
#include <QColor>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QThread>
#include <libtsm.h>



// ***********************************

#ifdef _WIN32
    #define ARROW_UP "\x48"
    #define ARROW_DOWN "\x50"
    #define ARROW_LEFT "\x4B"
    #define ARROW_RIGHT "\x4D"
    #define HOME_KEY "\x47"
    #define END_KEY "\x4F"
    #define PAGEUP_KEY "\x49"
    #define PAGEDOWN_KEY "\x51"
    #define F1_KEY "\x00\x3B"
    #define F2_KEY "\x00\x3C"
    #define F3_KEY "\x00\x3D"
    #define F4_KEY "\x00\x3E"
    #define F5_KEY "\x00\x3F"
    #define F6_KEY "\x00\x40"
    #define F7_KEY "\x00\x41"
    #define F8_KEY "\x00\x42"
    #define F9_KEY "\x00\x43"
    #define F10_KEY "\x00\x44"
    #define F11_KEY "\x00\x85"
    #define F12_KEY "\x00\x86"
#else
    #define ARROW_UP "\x1b[A"
    #define ARROW_DOWN "\x1b[B"
    #define ARROW_LEFT "\x1b[D"
    #define ARROW_RIGHT "\x1b[C"
    #define HOME_KEY "\x1b[H"
    #define END_KEY "\x1b[F"
    #define PAGEUP_KEY "\x1b[5~"
    #define PAGEDOWN_KEY "\x1b[6~"
    #define F1_KEY "\x1b[11~"
    #define F2_KEY "\x1b[12~"
    #define F3_KEY "\x1b[13~"
    #define F4_KEY "\x1b[14~"
    #define F5_KEY "\x1b[15~"
    #define F6_KEY "\x1b[17~"
    #define F7_KEY "\x1b[18~"
    #define F8_KEY "\x1b[19~"
    #define F9_KEY "\x1b[20~"
    #define F10_KEY "\x1b[21~"
    #define F11_KEY "\x1b[23~"
    #define F12_KEY "\x1b[24~"
#endif
// ***********************************

namespace qonsole {

    // ********************************
    struct q_selection {
        uint start_line;
        uint start_column;

        uint end_line;
        uint end_column;

        bool active = false;
    };

    struct q_palette {
        QColor black;
        QColor red;
        QColor green;
        QColor yellow;
        QColor blue;
        QColor magenta;
        QColor cyan;
        QColor white;
        QColor bright_black;
        QColor bright_red;
        QColor bright_green;
        QColor bright_yellow;
        QColor bright_blue;
        QColor bright_magenta;
        QColor bright_cyan;
        QColor bright_white;
        QColor selection_bg;
    };

    struct q_cursor_pos {
        uint x;
        uint y;
    };

    enum q_cursor_style {
        BLOCK,
        UNDERLINE,
        IBEAM,
        NONE
    };

    // ********************************

    namespace utils {
        // convert a Qt Key to ascii
        inline QByteArray qey2a(QKeyEvent*event) {
            if (event->modifiers() == Qt::ControlModifier) {
                if      (event->key() == Qt::Key_C) return "\x03";
                else if (event->key() == Qt::Key_B) return "\x02";
                else if (event->key() == Qt::Key_N) return "\x0E";
                else if (event->key() == Qt::Key_P) return "\x10";
                else if (event->key() == Qt::Key_O) return "\x0F";
                else if (event->key() == Qt::Key_F) return "\x06";
                else if (event->key() == Qt::Key_D) return "\x04";
                else                                return QByteArray();
            }
            else {
                if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) return "\n";
                else if (event->key() == Qt::Key_Tab)       return "\t";
                else if (event->key() == Qt::Key_Backspace) return "\x08";
                else if (event->key() == Qt::Key_Home)      return HOME_KEY;
                else if (event->key() == Qt::Key_End)       return END_KEY;
                else if (event->key() == Qt::Key_Left)      return ARROW_LEFT;
                else if (event->key() == Qt::Key_Up)        return ARROW_UP;
                else if (event->key() == Qt::Key_Right)     return ARROW_RIGHT;
                else if (event->key() == Qt::Key_Down)      return ARROW_DOWN;
                else if (event->key() == Qt::Key_PageUp)    return PAGEUP_KEY;
                else if (event->key() == Qt::Key_PageDown)  return PAGEDOWN_KEY;
                else if (event->key() == Qt::Key_F1)        return F1_KEY;
                else if (event->key() == Qt::Key_F2)        return F2_KEY;
                else if (event->key() == Qt::Key_F3)        return F3_KEY;
                else if (event->key() == Qt::Key_F4)        return F4_KEY;
                else if (event->key() == Qt::Key_F5)        return F5_KEY;
                else if (event->key() == Qt::Key_F6)        return F6_KEY;
                else if (event->key() == Qt::Key_F7)        return F7_KEY;
                else if (event->key() == Qt::Key_F8)        return F8_KEY;
                else if (event->key() == Qt::Key_F9)        return F9_KEY;
                else if (event->key() == Qt::Key_F10)       return F10_KEY;
                else if (event->key() == Qt::Key_F11)       return F11_KEY;
                else if (event->key() == Qt::Key_F12)       return F12_KEY;
                else return event->text().toUtf8();
            }
        }
    }  // end of utils

    // threaded reader
    class QonsoleReader : public QThread {
        Q_OBJECT

        std::atomic<bool> __running = false;

        public:

            #if defined(__linux__) || defined(__APPLE__)
            const int file_descriptor = 0;

            #elif defined(_WIN32)
            const union {
                HANDLE handle = INVALID_HANDLE_VALUE;     // for file handles
                SOCKET socket = INVALID_SOCKET;      // for socket handles
            };
            bool is_socket = false; // flag to identify handle type
            #endif

            // unix-like constructor
            #if defined(__linux__) || defined(__APPLE__)
            QonsoleReader(int fd) : file_descriptor(fd) {}

            #elif defined(_WIN32)
            // file handle constructor
            QonsoleReader(HANDLE h) {
                this->handle = h;
                this->is_socket = false;
            }
            // socket handle constructor
            QonsoleReader(SOCKET s) {
                this->socket = s;
                this->is_socket = true;
            }
            #endif

            // general
            bool is_running() {
                return __running.load();
            }

        protected:
            // each time try to read 32 bytes
            void run() override {
                if (is_running()) {
                    qDebug() << "reader already running!";
                    return;
                }

                qDebug() << "reader running ...";

                #if defined(__linux__) || defined(__APPLE__)
                qDebug() << "reading ...";

                // read and emit
                while (true) {
                    char buffer[BUFFER_SIZE];

                    ssize_t bytes_read = read(this->file_descriptor, buffer, BUFFER_SIZE);

                    if (bytes_read > 0) {
                        QByteArray data(buffer, bytes_read);

                        emit data_ready(data);

                    } else if (bytes_read == 0) {
                        qDebug() << "End of file reached.";
                        break;
                    } else {
                        qDebug() << "Error reading from file descriptor.";
                        break;
                    }
                }

                qDebug() << "done reading.";
                #elif defined(_WIN32)
                // check if using socket or file handle
                if (is_socket) {
                    qDebug() << "reading from socket:" << socket;
                    
                    while (__running) {
                        char buffer[BUFFER_SIZE];
                        int bytes_read = recv(socket, buffer, BUFFER_SIZE, 0);
                        
                        if (bytes_read > 0) {
                            QByteArray data(buffer, bytes_read);
                            emit data_ready(data);
                        } else if (bytes_read == 0) {
                            qDebug() << "Socket closed.";
                            break;
                        } else {
                            int error = WSAGetLastError();
                            if (error == WSAEWOULDBLOCK) {
                                QThread::msleep(10);
                                continue;
                            }
                            qDebug() << "Error reading from socket:" << error;
                            break;
                        }
                    }
                } else {
                    qDebug() << "reading from handle:" << handle;
                    
                    while (__running) {
                        char buffer[BUFFER_SIZE];
                        DWORD bytes_read = 0;
                        
                        if (ReadFile(handle, buffer, BUFFER_SIZE, &bytes_read, nullptr)) {
                            if (bytes_read > 0) {
                                QByteArray data(buffer, static_cast<int>(bytes_read));
                                emit data_ready(data);
                            }
                        } else {
                            DWORD error = GetLastError();
                            if (error == ERROR_BROKEN_PIPE || error == ERROR_HANDLE_EOF) {
                                qDebug() << "Pipe closed.";
                                break;
                            }
                            qDebug() << "Error reading from handle:" << error;
                            break;
                        }
                    }
                }
                
                qDebug() << "done reading.";
                #endif
                
                __running = false;
            }

            
        signals:
            // listen and when data ready emit this signal
            void data_ready(QByteArray data);
    };

    class QonsoleWidget : public QWidget {
        Q_OBJECT

        signals:
            void destructed(QonsoleReader*);
            void closed(QonsoleReader*);

        protected:
            QonsoleReader *reader = nullptr;

            QFont m_font;
            int m_char_width;
            int m_char_height;
            int m_cols = 80;
            int m_lines = 24;
            bool m_use_bold = false;
            bool m_is_selecting = false;  // inner state
            bool m_draw_empty_cells = false;
            bool __m_requesting_dump = false;

            QString m_screen_content;

            QColor m_default_fg;
            QColor m_default_bg;
            QColor m_selection_bg;
            QColor m_palette[16];

            struct q_draw_context {
                QPainter* painter;
                QonsoleWidget* widget;
            };

            q_cursor_style m_qcstyle = q_cursor_style::BLOCK;
            q_cursor_pos m_cursor_pos {0, 0};
            q_selection m_selection {0, 2, 0, 9, true};

            // tsm
            struct tsm_screen* m_screen;
            struct tsm_vte* m_vte;

            void closeEvent(QCloseEvent *event) {
                emit closed(reader);
            }

            void paintEvent(QPaintEvent *event) override {
                QPainter painter(this);

                draw_screen(painter);
                
                if (!m_screen)
                    return;
                
                draw_cursor(painter);
                
                painter.setFont(m_font);
                
                q_draw_context ctx;
                ctx.painter = &painter;
                ctx.widget = this;
                
                tsm_screen_draw(m_screen, draw_callback, &ctx);
            }
            
            void keyPressEvent(QKeyEvent *event) override {
                reset_selection();

                // translate QKeyEvent to terminal input sequences
                QByteArray sequence = qonsole::utils::qey2a(event);

                if (!sequence.isEmpty()) {
                    write_to_source(sequence);
                    update_cursor_pos();
                }
            }

            void mousePressEvent(QMouseEvent *event) {
                reset_selection();

                m_selection.active = true;
                px2pos(event->pos(), m_selection.start_column, m_selection.start_line);

            }

            void mouseReleaseEvent(QMouseEvent *event) {
                m_selection.active = false;
                update();
                // qDebug() << this->selected_text();
            }

            void mouseMoveEvent(QMouseEvent *event) {
                if (m_selection.active) {
                    m_is_selecting = true;
                    px2pos(event->pos(), m_selection.end_column, m_selection.end_line);

                    update();
                }
            }
            
            void wheelEvent(QWheelEvent *event) {
                // TODO: handle scrolling: tsm_screen_scroll_down
                // TODO: use page up/down with keys
            }

            void update_cursor_pos() {
                m_cursor_pos.x = tsm_screen_get_cursor_x(m_screen);
                m_cursor_pos.y = tsm_screen_get_cursor_y(m_screen);
                update();
            }

            void update_metrics() {
                m_font.setStyleHint(QFont::TypeWriter);
                m_font.setFixedPitch(true);
                
                QFontMetrics fm(m_font);
                m_char_width = fm.horizontalAdvance('M');
                m_char_height = fm.height();
            }

            void draw_cursor(QPainter &painter) {
                if (m_qcstyle == q_cursor_style::NONE) {
                    return;
                }

                // If cursor style is IBEAM or UNDERLINE and widget does not have focus, do not draw cursor
                if ((
                        m_qcstyle == q_cursor_style::IBEAM || 
                        m_qcstyle == q_cursor_style::UNDERLINE
                    ) && !hasFocus()) {
                    return;
                }

                // otherwise draw full cursor, if has focus, if not draw only cursor border
                int x = m_cursor_pos.x * m_char_width;
                int y = m_cursor_pos.y * m_char_height;

                // if block cursor
                if (m_qcstyle == q_cursor_style::BLOCK) {
                    painter.fillRect(x, y, m_char_width, m_char_height, m_default_fg);
                }
                // if underline cursor
                else if (m_qcstyle == q_cursor_style::UNDERLINE) {
                    painter.fillRect(x, y + m_char_height - 2, m_char_width, 2, m_default_fg);
                }
                // if IBEAM cursor
                else if (m_qcstyle == q_cursor_style::IBEAM) {
                    painter.fillRect(x + (m_char_width / 2) - 1, y, 2, m_char_height, m_default_fg);
                }
            }

            void draw_screen(QPainter &painter) {
                painter.fillRect(rect(), m_default_bg);
            }

            bool is_selected(uint col, uint line) {
                if (!m_is_selecting) return false;

                int sl = m_selection.start_line;
                int sc = m_selection.start_column;
                int el = m_selection.end_line;
                int ec = m_selection.end_column;

                // Normalize selection (ensure (sl,sc) <= (el,ec))
                if (sl > el || (sl == el && sc > ec)) {
                    std::swap(sl, el);
                    std::swap(sc, ec);
                }

                // Now a single clean check
                if (line < sl || line > el)
                    return false;

                if (line == sl && col < sc)
                    return false;

                if (line == el && col > ec)
                    return false;

                return true;
            }

            static int draw_callback(
                struct tsm_screen* screen,
                uint64_t id,
                const uint32_t* ch,
                size_t len,
                unsigned int width,
                unsigned int posx,
                unsigned int posy,
                const struct tsm_screen_attr* attr,
                tsm_age_t age,
                void* data
            ) {
                q_draw_context* ctx = static_cast<q_draw_context*>(data);
                QPainter* painter = ctx->painter;
                QonsoleWidget* self = ctx->widget;
                
                if (!self) {
                    qDebug() << "tsm: invalid data passed.";
                    return 0;
                }

                bool iss = self->is_selected(posx, posy);

                // if not empty cell or it's selected then draw it otherwise do not
                // in case did not check if selecting, empty cells including spaces for some reason
                // (libtsm) won't be drawn at all
                if (len == 0 && !iss) {
                    if (!self->m_draw_empty_cells) {
                        return 0;
                    }
                }
                
                int x = posx * self->m_char_width;
                int y = posy * self->m_char_height;
                
                // Get colors
                QColor fg = self->m_default_fg;
                QColor bg = self->m_default_bg;
                
                if (attr->fccode >= 0 && attr->fccode < 16) {
                    fg = self->m_palette[attr->fccode];
                }
                if (attr->bccode >= 0 && attr->bccode < 16) {
                    bg = self->m_palette[attr->bccode];
                }
                
                // Handle inverse
                if (attr->inverse) {
                    std::swap(fg, bg);
                }
                
                // Draw background
                // if selection use selection background color
                if (iss) {
                    bg = self->m_selection_bg;
                }

                painter->fillRect(x, y, self->m_char_width * width, self->m_char_height, bg);
                
                // Draw character
                QString text(EMPTY_CELL_REPLACEMENT);
                
                if (len > 0) {
                    text = QString::fromUcs4(reinterpret_cast<const char32_t*>(ch), len);
                }
                
                QFont font = self->m_font;

                if (self->m_use_bold && attr->bold)
                    font.setBold(true);

                if (attr->underline)
                    font.setUnderline(true);
                
                painter->setFont(font);
                painter->setPen(fg);
                painter->drawText(x, y + self->m_char_height - 3, text);
                
                return 0;
            }

            static void write_callback(struct tsm_vte* vte, const char* u8, size_t len, void* data) {
                QonsoleWidget* self = static_cast<QonsoleWidget*>(data);

                if (self) {
                    QByteArray output(u8, static_cast<int>(len));
                    self->write_to_source(output);
                }
            }

            void on_data_ready(QByteArray data) {
                if (m_vte) {
                    tsm_vte_input(m_vte, data.constData(), data.size());

                    update_cursor_pos();
                }
            }

            void load_default_palette() {
                // Default theme: Credit to <https://draculatheme.com/>
                m_palette[0]  = QColor("#21222C"); // Black
                m_palette[1]  = QColor("#FF5555"); // Red
                m_palette[2]  = QColor("#50FA7B"); // Green
                m_palette[3]  = QColor("#F1FA8C"); // Yellow
                m_palette[4]  = QColor("#BD93F9"); // Blue
                m_palette[5]  = QColor("#FF79C6"); // Magenta
                m_palette[6]  = QColor("#8BE9FD"); // Cyan
                m_palette[7]  = QColor("#F8F8F2"); // White
                m_palette[8]  = QColor("#6272A4"); // Bright Black
                m_palette[9]  = QColor("#FF6E6E"); // Bright Red
                m_palette[10] = QColor("#69FF94"); // Bright Green
                m_palette[11] = QColor("#FFFFA5"); // Bright Yellow
                m_palette[12] = QColor("#D6ACFF"); // Bright Blue
                m_palette[13] = QColor("#FF92DF"); // Bright Magenta
                m_palette[14] = QColor("#A4FFFF"); // Bright Cyan
                m_palette[15] = QColor("#FFFFFF");  // Bright White
                
                m_default_fg = m_palette[7];
                m_default_bg = m_palette[0];
                m_selection_bg = QColor(255, 255, 255, 40);
            }

            void resize_vt() {
                if (m_cols < 1) m_cols = 1;
                if (m_lines < 1) m_lines = 1;
                
                if (m_screen) {
                    tsm_screen_resize(m_screen, m_cols, m_lines);
                }

                // if connected to a local pty, notify the process about size change
                #if defined(__linux__) || defined(__APPLE__)
                if (reader && reader->file_descriptor >= 0) {
                    struct winsize ws;
                    ws.ws_col = m_cols;
                    ws.ws_row = m_lines;
                    ws.ws_xpixel = 0;
                    ws.ws_ypixel = 0;
                    ioctl(reader->file_descriptor, TIOCSWINSZ, &ws);
                }
                #endif
            }

        public:
            QonsoleWidget(QWidget*parent) : QWidget(parent) {
                set_font(QFont("Monospace", 14));

                // assuming it is a free widget
                if (!parent) {
                    set_vt_size(80, 24);
                    widget_fit_vt_size();
                }

                // Initialize libtsm
                int ret = tsm_screen_new(&m_screen, nullptr, nullptr);
                if (ret < 0) {
                    qWarning() << "Failed to create tsm screen";
                    return;
                }

                ret = tsm_vte_new(&m_vte, m_screen, write_callback, this, nullptr, nullptr);
                if (ret < 0) {
                    qWarning() << "Failed to create tsm vte";
                    tsm_screen_unref(m_screen);
                    m_screen = nullptr;
                    return;
                }

                if (!parent) {
                    resize(800, 500);
                }

                setFocusPolicy(Qt::FocusPolicy::StrongFocus);

                load_default_palette();
            }

            ~QonsoleWidget() {
                emit destructed(reader);

                if (m_vte) {
                    tsm_vte_unref(m_vte);
                    m_vte = nullptr;
                }
                if (m_screen) {
                    tsm_screen_unref(m_screen);
                    m_screen = nullptr;
                }

                // reader not deleted, as its set from outside so user must handle that.
            }

            void px2pos(QPoint p, uint &col, uint &line) {
                col = p.x() / m_char_width;
                line = p.y() / m_char_height;
            }

            QPoint pos2px(uint col, uint line) {
                return QPoint(
                    col * m_char_width,
                    line * m_char_height
                );
            }

            void reset_selection() {
                m_is_selecting = false;
                m_selection = {0, 0, 0, 0, false};
            }

            // write to source fd/handle
            ssize_t write_to_source(const QByteArray& data) {
                #if defined(__linux__) || defined(__APPLE__)
                    if (reader && reader->file_descriptor > 0) {
                        ssize_t bytes_written = write(
                            reader->file_descriptor, data.constData(), data.size()
                        );
                        return bytes_written;
                    }
                #elif defined(_WIN32)
                    if (reader) {
                        if (reader->is_socket) {
                            // write to socket
                            int bytes_sent = send(reader->socket, data.constData(), data.size(), 0);
                            return bytes_sent;
                        } else {
                            // write to handle (pipe/file)
                            DWORD bytes_written = 0;
                            if (WriteFile(reader->handle, data.constData(), data.size(), &bytes_written, nullptr)) {
                                return bytes_written;
                            }
                        }
                    }
                #endif
                return -1; // error
            }

            void set_vt_size(uint cols, uint lines) {
                m_cols = cols;
                m_lines = lines;

                resize_vt();
            }

            // adjust widget size: adapt to vt size
            void widget_fit_vt_size() {
                if (m_screen) {
                    resize(m_cols * m_char_width, m_lines * m_char_height);
                }
                else {
                    qDebug() << "no vt screen";
                }
            }

            // adjust vt size: adapt to widget size
            void vt_fit_widget_size() {
                m_cols = width() / m_char_width;
                m_lines = height() / m_char_height;

                resize_vt();
            }

            void set_color_palette(const q_palette plt) {
                m_palette[0]  = plt.black;
                m_palette[1]  = plt.red;
                m_palette[2]  = plt.green;
                m_palette[3]  = plt.yellow;
                m_palette[4]  = plt.blue;
                m_palette[5]  = plt.magenta;
                m_palette[6]  = plt.cyan;
                m_palette[7]  = plt.white;
                m_palette[8]  = plt.bright_black;
                m_palette[9]  = plt.bright_red;
                m_palette[10] = plt.bright_green;
                m_palette[11] = plt.bright_yellow;
                m_palette[12] = plt.bright_blue;
                m_palette[13] = plt.bright_magenta;
                m_palette[14] = plt.bright_cyan;
                m_palette[15] = plt.bright_white;

                m_default_fg = m_palette[7];
                m_default_bg = m_palette[0];
                m_selection_bg = plt.selection_bg;
                
            }
            
            // configure font
            void set_font(QFont fnt) {
                m_font = fnt;
                update_metrics();
            }

            // init reader
            void set_reader(QonsoleReader* r) {
                this->reader = r;

                connect(reader, &QonsoleReader::data_ready, this, &QonsoleWidget::on_data_ready);

                reader->start();
            }

            // for optimization, empty cells are not drawn by default
            void set_draw_empty_cells(bool s) {
                m_draw_empty_cells = s;
            }

            void get_terminal_size(int& cols, int& lines) {
                cols = m_cols;
                lines = m_lines;
            }

            // return where selection starts and ends as a flat
            const q_selection& get_selection() {
                return m_selection;
            }

            QString get_selected_text() {
                QString text = dump_screen();

                if (!m_selection.active) return QString();

                QStringList lines = text.split('\n');

                if (m_selection.start_line >= (uint)lines.size() || sel.end_line >= (uint)lines.size())
                    return QString();

                // Single-line selection
                if (sel.start_line == sel.end_line) {
                    const QString &line = lines[sel.start_line];
                    uint start = qMin(sel.start_column, (uint)line.size());
                    uint end   = qMin(sel.end_column,   (uint)line.size());
                    if (end < start) std::swap(start, end);
                    return line.mid(start, end - start);
                }

                QStringList out;

                // First line (partial)
                {
                    const QString &line = lines[sel.start_line];
                    uint start = qMin(sel.start_column, (uint)line.size());
                    out << line.mid(start);
                }

                // Middle full lines
                for (uint i = sel.start_line + 1; i < sel.end_line; ++i)
                    out << lines[i];

                // Last line (partial)
                {
                    const QString &line = lines[sel.end_line];
                    uint end = qMin(sel.end_column, (uint)line.size());
                    out << line.left(end);
                }

                return out.join("\n");
            }

            QString dump_screen() {
                
            } 
            
            void set_cursor_style(q_cursor_style qcs) {
                m_qcstyle = qcs;
            }

            void set_bold(bool s) {
                m_use_bold = s;
            }
    };
} // end of qonsole
