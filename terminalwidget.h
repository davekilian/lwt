#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "shell.h"

#include <QWidget>

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
};

#endif // TERMINALWIDGET_H
