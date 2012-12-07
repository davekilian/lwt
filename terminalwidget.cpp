#include "terminalwidget.h"

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>

TerminalWidget::TerminalWidget(QWidget *parent) :
    QWidget(parent)
{
}

void TerminalWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    QBrush bg(QColor(0, 0, 50));

    p.fillRect(0, 0, width(), height(), bg);
}
