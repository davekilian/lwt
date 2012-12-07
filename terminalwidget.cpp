#include "terminalwidget.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>

TerminalWidget::TerminalWidget(QWidget *parent) :
    QWidget(parent)
{
    // Debug
    for (int i = 0; i < 20; ++i) {
        QString line = "";

        int indent = rand() % 20;
        for (int j = 0; j < indent; ++j)
            line += ' ';

        line += "all work and no play makes jack a dull boy\n";
        m_contents.append(line);
    }
}

const QString& TerminalWidget::contents() const
{
    return m_contents;
}

QString &TerminalWidget::contentsRef()
{
    return m_contents;
}

void TerminalWidget::setContents(const QString &val)
{
    m_contents = val;
}

void TerminalWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing
                   | QPainter::TextAntialiasing
                   | QPainter::SmoothPixmapTransform
                   | QPainter::HighQualityAntialiasing);

    QBrush bg(QColor(0, 0, 50));
    p.fillRect(0, 0, width(), height(), bg);

    const int FONT_HEIGHT = 11;
    QFont font("Consolas", FONT_HEIGHT);
    font.setHintingPreference(QFont::PreferFullHinting);

    p.setFont(font);
    p.setPen(QColor(200, 200, 200));

    QFontMetrics fm(font);
    int y = fm.lineSpacing();
    int idx = 0;

    while (idx > -1 && idx < m_contents.length())
    {
        int beg = idx;
        int end = m_contents.indexOf("\n", beg + 1);
        if (end < 0) end = m_contents.length();

        p.drawText(0, y, m_contents.mid(beg, end - beg));

        idx = end;
        y += fm.lineSpacing();
    }
}
