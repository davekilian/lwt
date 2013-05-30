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

    const QString& contents() const;
    QString &contentsRef();
    void setContents(const QString &);

protected:
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void onshell(const QByteArray &data);
    void onshellexit();

private:
    QString m_contents;
    Shell *m_shell;
};

#endif // TERMINALWIDGET_H
