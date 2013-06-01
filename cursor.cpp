
#include "cursor.h"
#include "terminalwidget.h"

Cursor::Cursor(TerminalWidget *parent)
    : m_parent(parent),
      m_row(0),
      m_col(0),
      m_blinkVisible(true),
      m_blinkOn(750),
      m_blinkOff(750),
      m_blinkPause(1000)
{ 
    m_blinkTimer.setSingleShot(true);
    connect(&m_blinkTimer, SIGNAL(timeout()), this, SLOT(onBlinkTimer()));

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

static int indexOf(int row, int col, const QString &str)
{
    // Find the right line
    int index = 0;
    for (int i = 0; i < row; ++i)
    {
        index = str.indexOf('\n', index + 1);
        if (index == -1)
        {
            // The cursor is below the final line of text
            return -1;
        }
    }

    // Find the right column within the line
    int len = str.indexOf('\n', index + 1) - 1;
    if (len == -2)
        len = str.length();
    len -= index;

    if (col >= len)
    {
        // The cursor is beyond the end of the text on this line
        return -1;
    }

    if (row > 0)
        ++index;    // Skip the leading newline

    return index + col;
}

void Cursor::render(QPainter &p)
{
    // Do nothing if the cursor currently isn't blinking
    if (!m_blinkVisible)
        return;

    // Find the character under the cursor
    int index = indexOf(m_row, m_col, m_parent->contents());
    QChar c = index == -1 ? ' ' : m_parent->contents()[index];

    // Find out where the character is
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
    p.drawText(x, y + fm.lineSpacing(), QString(c));
}

void Cursor::onBlinkTimer()
{
    if (m_blinkVisible)
        beginOffBlink();
    else
        beginOnBlink();
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

