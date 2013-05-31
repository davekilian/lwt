#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "cursor.h"
#include "shell.h"

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

    /** Gets this terminal's entire input history */
    const QString& contents() const;

    /** Sets the termainl's input history to the given string */
    void setContents(const QString &);

    /** Gets a reference to the terminal's input history buffer */
    QString &buffer();

protected:
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void onShellRead(const QByteArray &data);
    void onShellExited();

private:
    QString m_contents;
    Shell *m_shell;
    Cursor m_cursor;
};

#endif // TERMINALWIDGET_H
