#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "cursor.h"
#include "history.h"
#include "shell.h"
#include "specialchars.h"
#include "theme.h"

#include <QLayout>
#include <QScrollBar>
#include <QWidget>

// TODO replace this with a configurable theming system
#define TERMINAL_FONT_FAMILY    "Andale Mono"
#define TERMINAL_FONT_HEIGHT    12

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

    /** Gets the foreground or background color of the cell at the given row
     *  and column (in cursor coordinates)
     */
    QColor foregroundColorAt(int row, int col) const;
    QColor backgroundColorAt(int row, int col) const;

protected:
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *);

private slots:
    void onShellRead(const QByteArray &data);
    void onShellExited();

    void onScroll(int);
    void onHistoryScrollToBottom();

    void doBell();
    void doSetCursorVisible(bool visible);
    void doSetWindowTitle(const QString &title);

private:
    History m_history;
    Shell *m_shell;
    Cursor m_cursor;
    SpecialChars m_chars;
    Theme m_theme;

    QLayout *m_layout;
    QScrollBar *m_scrollBar;

    void calcScrollbarSize();
    void scrollToEnd();
};

#endif // TERMINALWIDGET_H
