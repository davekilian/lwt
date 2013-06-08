#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "cursor.h"
#include "history.h"
#include "shell.h"
#include "specialchars.h"

#include <QLayout>
#include <QScrollBar>
#include <QWidget>

// TODO replace this with a configurable theming system
#define TERMINAL_FONT_FAMILY    "Andale Mono"
#define TERMINAL_FONT_HEIGHT    12
#define TERMINAL_BG_R           7
#define TERMINAL_BG_G           54
#define TERMINAL_BG_B           66
#define TERMINAL_FG_R           238
#define TERMINAL_FG_G           232
#define TERMINAL_FG_B           213

/** Fills the primary terminal window, rendering terminal output
  * text and accepting user input
  */
class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = 0);
    ~TerminalWidget();

    /** Gets the object that tracks the input history */
    const History &history() const;

    /** Gets or sets the amount the view has been scrolled, in pixels */
    int scrollAmount();
    void setScrollAmount(int);

protected:
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);

private slots:
    void onShellRead(const QByteArray &data);
    void onShellExited();

    void onScroll(int);

    void doBell();
    void doSetCursorVisible(bool visible);
    void doSetWindowTitle(const QString &title);

private:
    History m_history;
    Shell *m_shell;
    Cursor m_cursor;
    SpecialChars m_chars;

    QLayout *m_layout;
    QScrollBar *m_scrollBar;

    void calcScrollbarSize();
    void scrollToEnd();
};

#endif // TERMINALWIDGET_H
