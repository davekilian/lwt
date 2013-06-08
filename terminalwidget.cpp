#include "terminalwidget.h"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>

TerminalWidget::TerminalWidget(QWidget *parent) :
    QWidget(parent),
    m_cursor(this),
    m_layout(new QHBoxLayout),
    m_scrollBar(new QScrollBar)
{
    // Set up the scroll bar
    ((QHBoxLayout*)m_layout)->addWidget(m_scrollBar, 0, Qt::AlignRight);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    m_scrollBar->setTracking(true);
    connect(m_scrollBar, SIGNAL(sliderMoved(int)), this, SLOT(onScroll(int)));

    // Set up handlers for escape sequences received from the shell
    connect(&m_chars, SIGNAL(bell()), SLOT(doBell()));
    connect(&m_chars, SIGNAL(setCursorVisible(bool)),
                      SLOT(doSetCursorVisible(bool)));
    connect(&m_chars, SIGNAL(setWindowTitle(const QString&)), 
                      SLOT(doSetWindowTitle(const QString&)));

    // Set up handlers for history events
    connect(&m_history, SIGNAL(cursorMoved(int, int)),
            &m_cursor,  SLOT(moveTo(int, int)));
    connect(&m_history, SIGNAL(updated()),
                        SLOT(update()));

    // Debug
    QStringList a;
    a.append("--login");
    a.append("-i");
    m_shell = new Shell("C:\\Program Files (x86)\\Git\\bin\\sh.exe", a);
    connect(m_shell, SIGNAL(read(QByteArray)), SLOT(onShellRead(QByteArray)));
    connect(m_shell, SIGNAL(closed()), SLOT(onShellExited()));
    m_shell->open();
}

TerminalWidget::~TerminalWidget() 
{ 
    delete m_scrollBar;
    delete m_layout;
}

const History& TerminalWidget::history() const
{
    return m_history;
}

int TerminalWidget::scrollAmount()
{
    return m_scrollBar->value();
}

void TerminalWidget::setScrollAmount(int val)
{
    m_scrollBar->setValue(val);
}

void TerminalWidget::onShellRead(const QByteArray &data)
{
    m_history.beginInsert();

    QString input(data);
    while (input.length() > 0)
    {
        if (!m_chars.eat(&input))
        {
            m_history.insert(input[0]);
            input = input.right(input.length() - 1);
        }
    }

    m_history.endInsert();

    calcScrollbarSize();
    scrollToEnd();
}

void TerminalWidget::keyPressEvent(QKeyEvent *ev)
{
    m_shell->write(m_chars.translate(ev));
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
    QStringList lines = m_history.visibleLines(m_scrollBar->value(),
                                               m_scrollBar->value() + height(),
                                               fm.lineSpacing());

    int y = fm.lineSpacing() - (m_scrollBar->value() % fm.lineSpacing());
    foreach (const QString &line, lines)
    {
        p.drawText(0, y, line);
        y += fm.lineSpacing();
    }

    m_cursor.render(p);
}

void TerminalWidget::resizeEvent(QResizeEvent *)
{
    m_history.onViewportResized(width(), height());
    calcScrollbarSize();
    update();
}

void TerminalWidget::wheelEvent(QWheelEvent *we)
{
    int delta = 0;

    if (!we->pixelDelta().isNull())
        delta = we->pixelDelta().y();
    else
        delta = QApplication::wheelScrollLines() 
              * we->angleDelta().y() / 120 
              * m_scrollBar->singleStep();

    setScrollAmount(scrollAmount() - delta);
    calcScrollbarSize();
    update();
}

void TerminalWidget::onShellExited()
{
    window()->close();
}

void TerminalWidget::onScroll(int)
{
    calcScrollbarSize();
    update();
}

void TerminalWidget::calcScrollbarSize()
{
    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    QFontMetrics fm(font);

    int nLines = m_history.numLines();
    int contentHeight = nLines * fm.lineSpacing();

    m_scrollBar->setMinimum(0);
    m_scrollBar->setMaximum(contentHeight);
    m_scrollBar->setPageStep(height());
    m_scrollBar->setSingleStep(fm.lineSpacing());

    m_scrollBar->setVisible(contentHeight > height());
}

void TerminalWidget::scrollToEnd()
{
    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    QFontMetrics fm(font);

    int nLines = m_history.numLines();
    int contentHeight = nLines * fm.lineSpacing();

    int minValue = contentHeight - height() + 3 * fm.lineSpacing();
    if (scrollAmount() < minValue)
        setScrollAmount(minValue);
}

void TerminalWidget::doBell() 
{ 
    QApplication::beep();

    // TODO this doesn't seem to work? Maybe it only invokes the piezoelectric
    // bell on the motherboard (which my computer doesn't have)
    // There doesn't seem to be a good platform-agnostic way to play a system
    // alert sound in Qt then...
}

void TerminalWidget::doSetCursorVisible(bool visible)
{
    if (visible)
        m_cursor.show();
    else
        m_cursor.hide();
}

void TerminalWidget::doSetWindowTitle(const QString &title)
{
    ((QWidget*)parent()->parent())->setWindowTitle(title);
}

