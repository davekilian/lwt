
#include "history.h"

History::History() 
    : m_cursorLine(0),
      m_cursorCol(0),
      m_numRowsVisible(0),
      m_numColsVisible(0),
      m_viewportTop(0),
      m_viewportBottom(0),
      m_cellWidth(0),
      m_cellHeight(0)
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
    connect(chars, SIGNAL(resetColors()), SLOT(resetColors()));
    connect(chars, SIGNAL(scroll(int)), SLOT(scroll(int)));
    connect(chars, SIGNAL(setColor(SpecialColors::Color, bool, bool)), 
                     SLOT(setColor(SpecialColors::Color, bool, bool)));
    connect(chars, SIGNAL(setColor256(int, bool)), 
                     SLOT(setColor256(int, bool)));
    connect(chars, SIGNAL(verticalTab()), SLOT(verticalTab()));
}

void History::beginWrite() { }

void History::write(QChar c)
{
    Q_ASSERT(m_cursorLine < m_vlines.size());

    vline &v = m_vlines[m_cursorLine];
    Q_ASSERT(m_cursorCol <= v.len);

    int lineNumber = v.line;

    // Handle a newline
    if (c == '\n')
    {
        ++m_cursorLine;
        m_cursorCol = 0;

        if (m_cursorLine == m_vlines.size())
        {
            m_lines.append("");
            m_vlines.append(vline(m_lines.size() - 1, 0, 0));
        }
    }

    // Overwrite an old character
    else if (m_cursorCol < v.len)
    {
        int cursorCol = v.beg + m_cursorCol;
        m_lines[lineNumber][cursorCol] = c;

        ++m_cursorCol;
    }

    // Append a new character
    else
    {
        int cursorCol = v.beg + m_cursorCol;
        m_lines[lineNumber].insert(cursorCol, c);

        ++m_cursorCol;
        ++v.len;
    }
}

void History::write(const vline &line)
{
    const QString &str = m_lines[line.line];

    for (int i = 0; i < line.len; ++i)
        write(str[line.beg + i]);
}

void History::writeBlanks(int count)
{
    for (int i = 0; i < count; ++i)
        write(' ');
}

void History::endWrite()
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

QStringList History::visibleLines() const
{
    int min = m_viewportTop / m_cellHeight,     // Row at top of viewport
        max = m_viewportBottom / m_cellHeight;  // Row at bottom of viewport

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

void History::onViewportChanged(int yTop, int yBottom, int lineHeight, 
                                int width, int averageCharWidth)
{
    m_cellWidth = averageCharWidth;
    m_cellHeight = lineHeight;
    m_viewportTop = yTop;
    m_viewportBottom = yBottom;

    int numRowsVisible = (yBottom - yTop) / lineHeight,
        numColsVisible = width / averageCharWidth;

    bool resized = numRowsVisible != m_numRowsVisible ||
                   numColsVisible != m_numColsVisible;

    m_numRowsVisible = numRowsVisible;
    m_numColsVisible = numColsVisible;

    if (resized)
    {
        wrapLines();

        emit cursorMoved(m_cursorLine, m_cursorCol);
        emit updated();
    }
}

void History::backspace()
{
    if (m_cursorCol == 0)
        return; // Nothing before the cursor

    Q_ASSERT(m_cursorLine < m_vlines.size());

    const vline &v = m_vlines[m_cursorLine];
    Q_ASSERT(m_cursorCol <= v.len);

    m_lines[v.line].remove(v.beg + m_cursorCol - 1, 1);
    --m_cursorCol;

    wrapLines();

    emit cursorMoved(m_cursorLine, m_cursorCol);
    emit updated();
}

void History::carriageReturn()
{
    m_cursorCol = 0;
    emit cursorMoved(m_cursorLine, m_cursorCol);
}

void History::del()
{
    Q_ASSERT(m_cursorLine < m_vlines.size());

    const vline &v = m_vlines[m_cursorLine];
    Q_ASSERT(m_cursorCol <= v.len);

    m_lines[v.line].remove(v.beg + m_cursorCol, 1);

    wrapLines();
    emit updated();
}

void History::erase(SpecialChars::EraseType type)
{
    // The spec-correct behavior here is to actually delete data from m_lines
    // and then recompute m_vlines. However, Ctrl+L causes MinGW bash to
    // produce a sequence that erases all history data. I'd rather Ctrl+L not
    // be destructive, so I'm treating this like a form-feed++ event.
    //
    // The basic idea is to form-feed and then re-add the data that wasn't
    // supposed to be erased. Unfortunately, this is kinda messy :)

    // Store information that will be destroyed once we start echoing lines
    int curLine         = m_cursorLine,
        curCol          = m_cursorCol,
        curDistFromEnd  = m_vlines.size() - 1 - m_cursorLine,
        beg             = m_vlines.size() - 1 - m_numRowsVisible,
        end             = m_vlines.size() - 1;

    if (beg < 0)
        beg = 0;

    // Make sure the cursor is below any visible data
    formFeed();

    // Re-print any data that wasn't supposed to have been deleted
    for (int i = beg; i <= end; ++i)
    {
        // If this line came before the cursor ...
        if (i < curLine)
        {
            // ... the line should get echoed unless we're erasing the screen
            // before the cursor or just the entire screen
            if (type != SpecialChars::ERASE_SCREEN &&
                type != SpecialChars::ERASE_SCREEN_BEFORE)
            {
                write(m_vlines[i]);
            }

            write('\n');
        }

        // If this line contains the cursor ...
        if (i == curLine)
        {
            vline before(m_vlines[i]),
                  after(m_vlines[i]);

            before.len = curCol;
            after.beg = curCol;
            after.len = m_vlines[i].len - after.beg;

            // ... the part of the line before the cursor should be echoed
            // only if we're deleting things after the cursor
            if (type == SpecialChars::ERASE_LINE_AFTER ||
                type == SpecialChars::ERASE_SCREEN_AFTER)
            {
                write(before);
            }
            else
            {
                writeBlanks(before.len);
            }

            // ... the part of the line after the cursor should be echoed only
            // if we're deleting things before the cursor
            if (type == SpecialChars::ERASE_LINE_BEFORE ||
                type == SpecialChars::ERASE_SCREEN_BEFORE)
            {
                write(after);
                write('\n');
            }
        }

        // If this line came after the cursor ...
        if (i > curLine)
        {
            // ... the line should get echoed unless we're erasing the screen
            // after the cursor or just the entire screen
            if (type != SpecialChars::ERASE_SCREEN &&
                type != SpecialChars::ERASE_SCREEN_AFTER)
            {
                write(m_vlines[i]);
                write('\n');
            }
        }
    }

    // Move the cursor relative to where it was before we echoed old lines
    m_cursorCol = curCol;

    if (type != SpecialChars::ERASE_SCREEN &&
        type != SpecialChars::ERASE_SCREEN_AFTER)
    {
        // More text was printed after the line the cursor is supposed to be
        // on, so move the cursor back to that line.
        m_cursorLine = m_vlines.size() - 1 - curDistFromEnd;
    }
}

void History::formFeed()
{
    scroll(1);
}

void History::horizontalTab()
{
    // To implement horizontal tabs, it'd be sufficient to just append a \t
    // character here. Instead, we manually convert the tab to spaces to keep
    // the invariant that every character in the input buffer occupies exactly
    // one cell on the screen.

    const int SPACES_PER_TAB = 8;
    int nspaces = SPACES_PER_TAB - (m_cursorCol % SPACES_PER_TAB);

    for (int i = 0; i < nspaces; ++i)
        write(' ');
}

void History::moveCursorBy(int rowDelta, int colDelta)
{
    moveCursorTo(m_cursorLine + rowDelta, m_cursorCol + colDelta);
}

void History::moveCursorTo(int row, int col)
{
    if (row >= 0)
    {
        m_cursorLine = m_vlines.size() - 1
                     - m_numRowsVisible
                     + row;

        if (m_cursorLine < 0)
            m_cursorLine = 0;
        else if (m_cursorLine >= m_vlines.size())
            m_cursorLine = m_vlines.size() - 1;
    }

    if (col >= 0)
    {
        m_cursorCol = col;

        const vline &v = m_vlines[m_cursorLine];
        if (m_cursorCol < 0)
            m_cursorCol = 0;
        else if (m_cursorCol > v.len)
            m_cursorCol = v.len;
    }

    emit cursorMoved(m_cursorLine, m_cursorCol);
}

void History::resetColors()
{
    // TODO
}

void History::scroll(int npages)
{
    // TODO support -npages values. What's the desired behavior exactly?

    for (int i = 0; i < npages * m_numRowsVisible; ++i)
        write('\n');

    emit scrollToBottom();
}

void History::setColor(SpecialChars::Color c, bool bright, bool foreground)
{
    (void)c;
    (void)bright;
    (void)foreground;
    // TODO
}

void History::setColor256(int index, bool foreground)
{
    (void)index;
    (void)foreground;
    // TODO
}

void History::verticalTab()
{
    for (int i = 0; i < 3; ++i)
        write('\t');
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
