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

TerminalWidget::TerminalWidget(QWidget *parent) 
    : QWidget(parent),
      m_shell(Shell::create()),
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
    connect(&m_history, SIGNAL(scrollToBottom()),
                        SLOT(onHistoryScrollToBottom()));

    m_history.connectTo(&m_chars);

    // Set up handlers for shell events
    connect(m_shell, SIGNAL(read(QString)), SLOT(onShellRead(QString)));
    connect(m_shell, SIGNAL(closed()), SLOT(onShellExited()));
    m_shell->open();
}

TerminalWidget::~TerminalWidget() 
{ 
    delete m_shell;
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

QColor TerminalWidget::foregroundColorAt(int row, int col) const
{
    return m_theme.color(m_history.foregroundColorAt(row, col));
}

QColor TerminalWidget::backgroundColorAt(int row, int col) const
{
    return m_theme.color(m_history.backgroundColorAt(row, col));
}

void TerminalWidget::onShellRead(const QString &input)
{
    m_history.beginWrite();

    int i = 0;
    while (i < input.length())
    {
        int next = m_chars.eat(input, i);
        if (next == i)
        {
            m_history.write(input[i++]);
        }
        else
        {
            i = next;
        }
    }

    m_history.endWrite();

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

    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    font.setHintingPreference(QFont::PreferFullHinting);
    p.setFont(font);

    // Fill the background
    QBrush bg(m_theme.color(0));
    p.fillRect(0, 0, width(), height(), bg);

    // Draw text
    QFontMetrics fm(font);
    RenderData rd = m_history.renderData(m_scrollBar->value(),
                                         m_scrollBar->value() + height(),
                                         fm.lineSpacing());

    int y = fm.lineSpacing() - (m_scrollBar->value() % fm.lineSpacing());
    rd.begin();

    while (rd.nextLine())
    {
        RenderData::Section section;
        int x = 0;

        while (rd.next(&section))
        {
            if (section.background != 0)
            {
                p.fillRect(x, y,
                           section.data.size() * fm.averageCharWidth(),
                           fm.lineSpacing(),
                           QBrush(m_theme.color(section.background)));
            }

            p.setPen(m_theme.color(section.foreground));
            p.drawText(x, y, section.data);
            x += fm.averageCharWidth() * section.data.size();
        }

        y += fm.lineSpacing();
    }

    rd.end();

    // Draw the cursor, if applicable
    m_cursor.render(p);
}

void TerminalWidget::resizeEvent(QResizeEvent *)
{
    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    QFontMetrics fm(font);

    int w = width() - (m_scrollBar->isVisible() ? m_scrollBar->width() : 0),
        h = height(),
        numRows = h / fm.lineSpacing(),
        numCols = w / fm.averageCharWidth();

    m_history.onViewportResized(numRows, numCols);

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

void TerminalWidget::onHistoryScrollToBottom()
{
    calcScrollbarSize();
    m_scrollBar->setValue(m_scrollBar->maximum());
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

    int minValue = contentHeight - height() + fm.lineSpacing();
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
