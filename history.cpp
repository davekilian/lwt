
#include "history.h"

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
    connect(chars, SIGNAL(carriageReturn()), SLOT(carriageReturn()));
    connect(chars, SIGNAL(del(int)), SLOT(del(int)));
    connect(chars, SIGNAL(erase(SpecialChars::EraseType)), 
                     SLOT(erase(SpecialChars::EraseType)));
    connect(chars, SIGNAL(formFeed()), SLOT(formFeed()));
    connect(chars, SIGNAL(horizontalTab()), SLOT(horizontalTab()));
    connect(chars, SIGNAL(insert(int)), SLOT(insert(int)));
    connect(chars, SIGNAL(moveCursorBy(int, int)), 
                     SLOT(moveCursorBy(int, int)));
    connect(chars, SIGNAL(moveCursorTo(int, int)), 
                     SLOT(moveCursorTo(int, int)));
    connect(chars, SIGNAL(resetColors()), SLOT(resetColors()));
    connect(chars, SIGNAL(setColor(SpecialChars::Color, bool, bool)),
                     SLOT(setColor(SpecialChars::Color, bool, bool)));
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

RenderData History::renderData(int yTop, int yBottom, int lineHeight) const
{
    int minLine = yTop / lineHeight,
        maxLine = yBottom / lineHeight;

    if (minLine >= m_vlines.size())
        return RenderData(QVector<RenderData::Section>());

    // Scan through gevents to find ...
    int fg = 7,     // ... the current foreground color
        bg = 0,     // ... the current background color
        gindex = 0; // ... the index of the next gevent

    for (int i = 0; i < m_gevents.size(); ++i)
    {
        const gevent &g = m_gevents[i];
        const vline  &v = m_vlines[minLine];

        if (g.line > v.line ||
           (g.line == v.line && g.col > v.beg))
        {
            // This is the first gevent that occurs after the first visible
            // character -- done scanning
            gindex = i;
            break;
        }
        else
        {
            if (g.foreground)
                fg = g.color;
            else
                bg = g.color;
        }
    }

    // Walk through the visible lines, splitting them up into color sections
    QVector<RenderData::Section> sections;

    for (int i = minLine; i < maxLine && i < m_vlines.size(); ++i)
    {
        const vline &v = m_vlines[i];
        int vindex = v.beg;

        // While there are unprocessed graphics changes within this vline,
        // subdivide the vline and process the graphics changes
        while (gindex < m_gevents.size() &&
               m_gevents[gindex].line == v.line &&
               m_gevents[gindex].col <= v.beg + v.len)
        {
            RenderData::Section s;
            s.line = i;
            s.foreground = fg;
            s.background = bg;

            gevent g = m_gevents[gindex];
            s.data = m_lines[v.line].mid(vindex, g.col - vindex);
            vindex = g.col;

            sections.append(s);

            while (g.col == vindex)
            {
                if (g.foreground)
                    fg = g.color;
                else
                    bg = g.color;

                ++gindex;
                if (gindex >= m_gevents.size())
                    break;

                g = m_gevents[gindex];
            }
        }

        // No more graphics changes in the vline -- process the rest of it
        if (vindex - v.beg <= v.len)
        {
            RenderData::Section s;
            s.line = i;
            s.data = m_lines[v.line].mid(vindex, v.len - (vindex - v.beg));
            s.foreground = fg;
            s.background = bg;

            sections.append(s);
        }
    }

    return RenderData(sections);
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

void History::carriageReturn()
{
    m_cursorCol = 0;
    emit cursorMoved(m_cursorLine, m_cursorCol);
}

void History::del(int n)
{
    Q_ASSERT(m_cursorLine < m_vlines.size());

    const vline &v = m_vlines[m_cursorLine];
    Q_ASSERT(m_cursorCol <= v.len);

    if (m_cursorCol + n > v.len)
        n = v.len - m_cursorCol;

    m_lines[v.line].remove(v.beg + m_cursorCol, n);
    m_vlines[m_cursorLine].len -= n;
}

void History::erase(SpecialChars::EraseType type)
{
    // The spec-correct behavior for this function is to actually delete data
    // from m_lines and m_vlines. However, MinGW bash currently produces a
    // sequence that would destructively erase all data when the user presses
    // Ctrl+L. That's kind of unacceptable.
    //
    // So far I've only seen this handler used for two things:
    // - Clearing the entire screen
    // - Removing part of the current line buffer
    //
    // So, those are the two things we'll support. We'll implement the former
    // case with a form-feed so that clearing the screen is non-destructive.
    //
    // Doing this properly in a non-destructive way is really hard btw :)
    // Check revision 3a655e7a5d463d01f9fcd05d4c0d8f9c99374123

    if (type == SpecialChars::ERASE_SCREEN ||
        type == SpecialChars::ERASE_SCREEN_BEFORE ||
        type == SpecialChars::ERASE_SCREEN_AFTER)
    {
        formFeed();
    }
    else
    {
        if (m_cursorLine != m_vlines.size() - 1)
        {
            // We only support editing on the last line. Implementing erasure
            // on arbitrary lines would be possible, but that plus word wrap
            // would probably interact weirdly with terminal widget's scrollbar
            // calculations.
            return;
        }

        int lineIndex = m_lines.size() - 1,
            vlineIndex = m_vlines.size() - 1;

        if (type == SpecialChars::ERASE_LINE)
        {
            m_lines[lineIndex] = "";
            m_vlines[vlineIndex].beg = 0;
            m_vlines[vlineIndex].len = 0;
        }
        else if (type == SpecialChars::ERASE_LINE_BEFORE)
        {
            QString &l = m_lines[lineIndex];
            vline &v = m_vlines[vlineIndex];

            l = l.left(v.beg)                           // Before vline
              + l.mid(v.beg + m_cursorCol,              // After cursor
                      v.len - m_cursorCol)
              + l.right(l.size() - (v.beg + v.len));    // After vline

            v.len -= m_cursorCol;
            m_cursorCol = 0;
        }
        else if (type == SpecialChars::ERASE_LINE_AFTER)
        {
            QString &l = m_lines[lineIndex];
            vline &v = m_vlines[vlineIndex];

            l = l.left(v.beg)                           // Before vline
              + l.mid(v.beg, m_cursorCol)               // Before cursor
              + l.right(l.size() - (v.beg + v.len));    // After vline

            v.len = m_cursorCol;
        }
    }
}

void History::formFeed()
{
    for (int i = 0; i < m_numRowsVisible; ++i)
        write('\n');

    emit scrollToBottom();
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

void History::insert(int n)
{
    // Get the vline to edit
    Q_ASSERT(m_cursorLine < m_vlines.size());

    const vline &v = m_vlines[m_cursorLine];
    Q_ASSERT(m_cursorCol <= v.len);

    // Add n blanks to the line starting at the cursor
    QString toInsert;
    for (int i = 0; i < n; ++i)
        toInsert += ' ';

    m_lines[v.line].insert(v.beg + m_cursorCol, toInsert);

    // Update v.beg for subsequent lines that share this line
    for (int i = m_cursorLine + 1;
         i < m_vlines.size() && m_vlines[i].line == v.line;
         ++i)
    {
        m_vlines[i].beg += n;
    }
}

void History::moveCursorBy(int rowDelta, int colDelta)
{
    int row = m_cursorLine + rowDelta - (m_vlines.size() - 1 - m_numRowsVisible);
    int col = m_cursorCol + colDelta;

    moveCursorTo(row, col);
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
    setColor(SpecialChars::DEFAULT, false, true);
    setColor(SpecialChars::DEFAULT, false, false);
}

void History::setColor(SpecialChars::Color c, bool bright, bool foreground)
{
    int color = (int)c;

    if (c == SpecialChars::DEFAULT)
    {
        if (foreground)
            color = (int)SpecialChars::WHITE;
        else
            color = (int)SpecialChars::BLACK;
    }

    if (bright)
        color += 8;

    setColor256(color, foreground);
}

void History::setColor256(int color, bool foreground)
{
    int line = m_vlines[m_cursorLine].line,
        col  = m_vlines[m_cursorLine].beg + m_cursorCol;

    gevent g(line, col, foreground, color);

    // Go through the list backwards, and insert after the first existing
    // gevent that is supposed to come before this gevent (We go backwards
    // because we expect most gevents to insert near the end of the buffer)
    int index = m_gevents.size();
    while (index - 1 >= 0 && m_gevents[index - 1].compareTo(g) > 0)
    {
        --index;
    }

    m_gevents.insert(index, g);
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

