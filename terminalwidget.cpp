#include "terminalwidget.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>

TerminalWidget::TerminalWidget(QWidget *parent) :
    QWidget(parent),
    m_cursor(this)
{
    // Debug
    QStringList a;
    a.append("--login");
    a.append("-i");
    m_shell = new Shell("C:\\Program Files (x86)\\Git\\bin\\sh.exe", a);
    connect(m_shell, SIGNAL(read(QByteArray)), SLOT(onShellRead(QByteArray)));
    connect(m_shell, SIGNAL(closed()), SLOT(onShellExited()));
    m_shell->open();
}

TerminalWidget::~TerminalWidget() { }

const QString& TerminalWidget::contents() const
{
    return m_contents;
}

void TerminalWidget::setContents(const QString &val)
{
    m_contents = val;
}

QString &TerminalWidget::buffer()
{
    return m_contents;
}

void TerminalWidget::onShellRead(const QByteArray &data)
{
    m_contents.append(data);

    // DEBUG move the cursor to the end of input
    // This eventually needs to be driven by control characters
    int row = 0, index = -1, next = 0;
    while ((next = m_contents.indexOf('\n', index + 1)) > -1)
    {
        ++row;
        index = next;
    }
    int col = m_contents.length() - 1 - index;
    m_cursor.moveTo(row, col);

    update();
}

void TerminalWidget::keyPressEvent(QKeyEvent *ev)
{
    // DEBUG Test the cursor
    if (ev->key() == Qt::Key_Up)
        m_cursor.moveBy(-1, 0);
    if (ev->key() == Qt::Key_Down)
        m_cursor.moveBy( 1, 0);
    if (ev->key() == Qt::Key_Left)
        m_cursor.moveBy(0, -1);
    if (ev->key() == Qt::Key_Right)
        m_cursor.moveBy(0,  1);
    update();
    
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

    QBrush bg(QColor(TERMINAL_BG_R, TERMINAL_BG_G, TERMINAL_BG_B));
    p.fillRect(0, 0, width(), height(), bg);

    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    font.setHintingPreference(QFont::PreferFullHinting);

    p.setFont(font);
    p.setPen(QColor(TERMINAL_FG_R, TERMINAL_FG_G, TERMINAL_FG_B));

    QFontMetrics fm(font);
    int y = fm.lineSpacing();
    int idx = 0;

    while (idx > -1 && idx < m_contents.length())
    {
        int beg = idx;
        if (idx > 0)
            ++beg;

        int end = m_contents.indexOf("\n", beg);
        if (end < 0) end = m_contents.length();

        p.drawText(0, y, m_contents.mid(beg, end - beg));

        idx = end;
        y += fm.lineSpacing();
    }

    m_cursor.render(p);
}

void TerminalWidget::onShellExited()
{
    window()->close();
}

