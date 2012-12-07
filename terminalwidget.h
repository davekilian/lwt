#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>

/** Fills the primary terminal window, rendering terminal output
  * text and accepting user input
  */
class TerminalWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TerminalWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *);
};

#endif // TERMINALWIDGET_H
