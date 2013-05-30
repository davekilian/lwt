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
    QStringList a;
    a.append("--login");
    a.append("-i");
    m_shell = new Shell("C:\\Program Files (x86)\\Git\\bin\\sh.exe", a);
    connect(m_shell, SIGNAL(read(QByteArray)), SLOT(onshell(QByteArray)));
    connect(m_shell, SIGNAL(closed()), SLOT(onshellexit()));
    m_shell->open();
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

void TerminalWidget::onshell(const QByteArray &data)
{
    m_contents.append(data);
    update();
}

void TerminalWidget::keyPressEvent(QKeyEvent *ev)
{
    QString text = ev->text().replace('\r', '\n');
    if (text.length() == 0)
        return;

    // Qt sends EOT instead of newlines when the user hits return
    for (int i = 0; i < text.length(); ++i)
        if (text[i] == 4)
            text[i] = '\n';

    m_shell->write(text);
}

void TerminalWidget::paintEvent(QPaintEvent *)
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

void TerminalWidget::onshellexit()
{
    window()->close();
}
