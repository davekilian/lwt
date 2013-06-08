
#include "history.h"

/* TODO give a breakdown of the algorithm */

History::History() 
    : m_cursorLine(0),
      m_cursorCol(0),
      m_numRowsVisible(0),
      m_numColsVisible(0)
{ 
    m_lines.append("");
    m_vlines.append(vline());
}

History::~History() { }

void History::connectTo(SpecialChars *chars) const
{
    connect(chars, SIGNAL(backspace()), SLOT(backspace()));
    connect(chars, SIGNAL(carriageReturn()), SLOT(carriageReturn()));
    connect(chars, SIGNAL(del()), SLOT(del()));
    connect(chars, SIGNAL(erase(SpecialChars::EraseType)), 
                     SLOT(erase(SpecialChars::EraseType)));
    connect(chars, SIGNAL(formFeed()), SLOT(formFeed()));
    connect(chars, SIGNAL(horizontalTab()), SLOT(horizontalTab()));
    connect(chars, SIGNAL(moveCursorBy(int, int)), 
                     SLOT(moveCursorBy(int, int)));
    connect(chars, SIGNAL(moveCursorTo(int, int)), 
                     SLOT(moveCursorTo(int, int)));
    connect(chars, SIGNAL(popCursorPosition()), SLOT(popCursorPosition()));
    connect(chars, SIGNAL(pushCursorPosition()), SLOT(pushCursorPosition()));
    connect(chars, SIGNAL(resetColors()), SLOT(resetColors()));
    connect(chars, SIGNAL(scroll(int)), SLOT(scroll(int)));
    connect(chars, SIGNAL(setColor(SpecialColors::Color, bool, bool)), 
                     SLOT(setColor(SpecialColors::Color, bool, bool)));
    connect(chars, SIGNAL(setColor256(int, bool)), 
                     SLOT(setColor256(int, bool)));
    connect(chars, SIGNAL(verticalTab()), SLOT(verticalTab()));
}

void History::beginInsert() { }

void History::insert(QChar c)
{
    // Bail if the cursor is somewhere whacky
    if (m_cursorLine >= m_vlines.size())
        return; // NYI -- or should this be an assert? TODO

    vline &v = m_vlines[m_cursorLine];
    if (m_cursorCol > v.len)
        return; // NYI -- or assert, TODO

    // Insert the character
    int lineNumber = v.line;

    if (c == '\n')
    {
        ++lineNumber;
        ++m_cursorLine;
        m_cursorCol = 0;

        m_lines.insert(lineNumber, "");
        m_vlines.insert(m_cursorLine, vline(lineNumber, 0, 0));
    }
    else
    {
        int cursorCol = v.beg + m_cursorCol;
        m_lines[lineNumber].insert(cursorCol, c);

        ++m_cursorCol;
        ++v.len;
    }
}

void History::endInsert()
{
    wrapLines();

    emit cursorMoved(m_cursorLine, m_cursorCol);
    emit updated();
}

QChar History::charAt(int row, int col) const
{
    if (row >= m_vlines.size())
        return ' ';

    const vline &v = m_vlines[row];
    if (col >= v.len)
        return ' ';

    const QString &l = m_lines[v.line];
    return l[v.beg + col];
}

QString History::line(int index) const
{
    if (index >= m_vlines.size())
        return QString();

    const vline &v = m_vlines[index];
    const QString &l = m_lines[v.line];

    return l.mid(v.beg, v.len);
}

QStringList History::visibleLines(int yTop, int yBottom, int lineHeight) const
{
    int min = yTop / lineHeight,        // Row at top of viewport
        max = yBottom / lineHeight;     // Row at bottom of viewport

    QStringList ret;
    for (int i = min; i < max && i < m_vlines.size(); ++i)
    {
        const vline &v = m_vlines[i];
        const QString &l = m_lines[v.line];

        ret.append(l.mid(v.beg, v.len));
    }

    return ret;
}

int History::numLines() const
{
    return m_vlines.size();
}

void History::onViewportResized(int numRowsVisible, int numColsVisible)
{
    m_numRowsVisible = numRowsVisible;
    m_numColsVisible = numColsVisible;

    wrapLines();

    emit cursorMoved(m_cursorLine, m_cursorCol);
    emit updated();
}

void History::backspace()
{
}

void History::carriageReturn()
{
}

void History::del()
{
}

void History::erase(SpecialChars::EraseType type)
{
    (void)type;
}

void History::formFeed()
{
}

void History::horizontalTab()
{
}

void History::moveCursorBy(int rowDelta, int colDelta)
{
    (void)rowDelta;
    (void)colDelta;
}

void History::moveCursorTo(int row, int col)
{
    (void)row;
    (void)col;
}

void History::popCursorPosition()
{
}

void History::pushCursorPosition()
{
}

void History::resetColors()
{
}

void History::scroll(int npages)
{
    (void)npages;
}

void History::setColor(SpecialChars::Color c, bool bright, bool foreground)
{
    (void)c;
    (void)bright;
    (void)foreground;
}

void History::setColor256(int index, bool foreground)
{
    (void)index;
    (void)foreground;
}

void History::verticalTab()
{
}

void History::wrapLines()
{
    // Record the canonical location of the cursor
    int cursorLine = m_vlines[m_cursorLine].line,
        cursorCol  = m_vlines[m_cursorLine].beg + m_cursorCol;

    // Replace the current vlines with a new list corresponding to the current
    // canonical line list
    m_vlines.clear();

    for (int i = 0; i < m_lines.size(); ++i)
    {
        const QString &line = m_lines[i];

        int next = 0;
        while (next <= line.size())
        {
            int count = qMin(m_numColsVisible, line.size() - next);
            vline v(i, next, count);

            m_vlines.append(v);

            // If this vline contains the cursor, update the cursor coords
            if (i == cursorLine &&
                next <= cursorCol &&
                next + count >= cursorCol)
            {
                m_cursorLine = m_vlines.size() - 1;
                m_cursorCol = cursorCol - v.beg;
            }

            next += m_numColsVisible;
        }
    }
}

