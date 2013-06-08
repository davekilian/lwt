
#include "cursor.h"
#include "terminalwidget.h"

Cursor::Cursor(TerminalWidget *parent)
    : m_parent(parent),
      m_row(0),
      m_col(0),
      m_hidden(false),
      m_blinkVisible(true),
      m_blinkOn(500),
      m_blinkOff(500),
      m_blinkPause(1000)
{ 
    m_blinkTimer.setSingleShot(true);
    connect(&m_blinkTimer, SIGNAL(timeout()), SLOT(onBlinkTimer()));
    connect(&m_hideTimer, SIGNAL(timeout()), SLOT(onHideTimer()));

    beginOnBlink();
}

Cursor::~Cursor() { }

int Cursor::row() const
{
    return m_row;
}

int Cursor::col() const
{
    return m_col;
}

void Cursor::show()
{
    m_hidden = false;
    m_parent->update();
}

void Cursor::hide(int ms)
{
    m_hidden = true;
    m_parent->update();

    m_hideTimer.setSingleShot(true);
    m_hideTimer.start(ms);
}

int Cursor::blinkOn() const
{
    return m_blinkOn;
}

void Cursor::setBlinkOn(int val)
{
    m_blinkOn = val;
}

int Cursor::blinkOff() const
{
    return m_blinkOff;
}

void Cursor::setBlinkOff(int val)
{
    m_blinkOff = val;
}

int Cursor::blinkPause() const
{
    return m_blinkPause;
}

void Cursor::setBlinkPause(int val)
{
    m_blinkPause = val;
}

void Cursor::moveTo(int row, int col)
{
    m_row = row;
    m_col = col;

    beginPauseBlink();
}

void Cursor::moveBy(int rowDelta, int colDelta)
{
    m_row += rowDelta;
    m_col += colDelta;

    beginPauseBlink();
}

void Cursor::render(QPainter &p)
{
    // Do nothing if the cursor currently isn't visible
    if (m_hidden || !m_blinkVisible)
        return;

    // Find out where the cursor is
    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    font.setHintingPreference(QFont::PreferFullHinting);

    QFontMetrics fm(font);
    int w = fm.averageCharWidth(),
        h = fm.lineSpacing(),
        x = m_col * w,
        y = m_row * h - m_parent->scrollAmount();

    // Draw the cursor itself
    QBrush fg(QColor(TERMINAL_FG_R, TERMINAL_FG_G, TERMINAL_FG_B));
    p.fillRect(x, y + fm.descent(), w, fm.ascent() + fm.descent(), fg);
    
    // Draw the inverted character the cursor is over
    p.setFont(font);
    p.setPen(QColor(TERMINAL_BG_R, TERMINAL_BG_G, TERMINAL_BG_B));

    QChar c = m_parent->history().charAt(m_row, m_col);
    p.drawText(x, y + fm.lineSpacing(), QString(c));
}

void Cursor::onBlinkTimer()
{
    if (m_blinkVisible)
        beginOffBlink();
    else
        beginOnBlink();
}

void Cursor::onHideTimer()
{
    show();
}

void Cursor::beginOnBlink()
{
    m_blinkVisible = true;
    m_parent->update();

    m_blinkTimer.start(m_blinkOn);
}

void Cursor::beginOffBlink()
{
    m_blinkVisible = false;
    m_parent->update();

    m_blinkTimer.start(m_blinkOff);
}

void Cursor::beginPauseBlink()
{
    m_blinkTimer.stop();

    m_blinkVisible = true;
    m_parent->update();

    m_blinkTimer.start(m_blinkPause);
}

