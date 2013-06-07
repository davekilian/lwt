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
    connect(&m_chars, SIGNAL(backspace()), SLOT(doBackspace()));
    connect(&m_chars, SIGNAL(carriageReturn()), SLOT(doCarriageReturn()));
    connect(&m_chars, SIGNAL(del()), SLOT(doDel()));
    connect(&m_chars, SIGNAL(erase(SpecialChars::EraseType)),
                      SLOT(doErase(SpecialChars::EraseType)));
    connect(&m_chars, SIGNAL(formFeed()), SLOT(doFormFeed()));
    connect(&m_chars, SIGNAL(horizontalTab()), SLOT(doHorizontalTab()));
    connect(&m_chars, SIGNAL(moveCursorBy(int, int)), 
                      SLOT(doMoveCursorBy(int, int)));
    connect(&m_chars, SIGNAL(moveCursorTo(int, int)), 
                      SLOT(doMoveCursorTo(int, int)));
    connect(&m_chars, SIGNAL(scroll(int)), SLOT(doScroll(int)));
    connect(&m_chars, SIGNAL(setCursorVisible(bool)),
                      SLOT(doSetCursorVisible(bool)));
    connect(&m_chars, SIGNAL(setWindowTitle(const QString&)), 
                      SLOT(doSetWindowTitle(const QString&)));
    connect(&m_chars, SIGNAL(verticalTab()), SLOT(doVerticalTab()));

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
    QString input(data);
    while (input.length() > 0)
    {
        if (!m_chars.eat(&input))
        {
            m_contents.append(input[0]);
            input = input.right(input.length() - 1);
        }
    }

    calcScrollbarSize();
    scrollToEnd();

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

    repaint();
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
    int y = fm.lineSpacing() - m_scrollBar->value();
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

void TerminalWidget::resizeEvent(QResizeEvent *)
{
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
    // TODO this should eventually be part of the input history object
    int nLines = 0;
    int tmp = -1;
    while ((tmp = m_contents.indexOf('\n', tmp + 1)) != -1)
        ++nLines;

    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    QFontMetrics fm(font);
    int contentHeight = nLines * fm.lineSpacing();

    m_scrollBar->setMinimum(0);
    m_scrollBar->setMaximum(contentHeight);
    m_scrollBar->setPageStep(height());
    m_scrollBar->setSingleStep(fm.lineSpacing());

    m_scrollBar->setVisible(contentHeight > height());
}

void TerminalWidget::scrollToEnd()
{
    // TODO this should eventually be part of the input history object
    int nLines = 0;
    int tmp = -1;
    while ((tmp = m_contents.indexOf('\n', tmp + 1)) != -1)
        ++nLines;

    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    QFontMetrics fm(font);
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

void TerminalWidget::doBackspace()
{
    // TODO once we implement the real input history object, forward this to
    // the object. This is clearly incorrect once the cursor moves at all :P

    m_contents = m_contents.left(m_contents.length() - 1);
    m_cursor.moveBy(0, -1);
    update();
}

void TerminalWidget::doCarriageReturn()
{
    doMoveCursorBy(0, -m_cursor.col());
}

void TerminalWidget::doDel()
{
    // TODO lol this is totally wrong, but we probably can't do anything more
    // until we have a history object with an arbitrary write pointer
    doBackspace();
}

void TerminalWidget::doErase(SpecialChars::EraseType)
{
    // TODO this is complicated if we want to act like gnome-terminal and not
    // cmd.exe (i.e. adding lines instead of permanently deleting the data)
    // Basically we need to measure stuff, call doScroll, and then replace some
    // of the text unless EraseType is ALL
    //
    // For now though ...
    doFormFeed();
}

void TerminalWidget::doFormFeed()
{
    doScroll(1);
}

void TerminalWidget::doHorizontalTab()
{
    // To implement horizontal tabs, it'd be sufficient to just append a \t
    // character here. Instead, we manually convert the tab to spaces to keep
    // the invariant that every character in the input buffer occupies exactly
    // one cell on the screen.

    // Get the last line
    // TODO this should be part of the input buffer object
    int beg = m_contents.lastIndexOf("\n") + 1;
    QString line = m_contents.right(m_contents.length() - beg);
    
    // Compute number of spaces to add
    int SPACES_PER_TAB = 8;
    int nspaces = SPACES_PER_TAB - (line.length() % SPACES_PER_TAB);

    // Add the spaces
    QString spaces;
    for (int i = 0; i < nspaces; ++i)
        spaces.append(' ');
    onShellRead(spaces.toUtf8());
}

void TerminalWidget::doMoveCursorBy(int rowDelta, int colDelta)
{
    if (rowDelta < -m_cursor.row())
        rowDelta = -m_cursor.row();

    if (colDelta < -m_cursor.col())
        colDelta = -m_cursor.col();

    m_cursor.moveBy(rowDelta, colDelta);
    update();

    // TODO once we implement the real input history object,
    //      this callback needs to set the write pointer.
    //
    //      in fact, it'd make sense for this method to just
    //      set the write position in the history buffer, and 
    //      have the history buffer raise a signal that actually
    //      moves the cursor
}

void TerminalWidget::doMoveCursorTo(int row, int col)
{
    if (row < 0)
        row = 0;
    if (col < 0)
        col = 0;

    m_cursor.moveTo(row, col);
    update();

    // TODO see the note in doMoveCursorBy()
}

void TerminalWidget::doScroll(int npages)
{
    QFont font(TERMINAL_FONT_FAMILY, TERMINAL_FONT_HEIGHT);
    QFontMetrics fm(font);

    int linesPerPage = height() / fm.lineSpacing() + 1;

    int totalLines = linesPerPage * npages;
    QString str;
    for (int i = 0; i < totalLines; ++i)
        str += "\n";

    onShellRead(str.toUtf8());

    m_scrollBar->setValue(m_scrollBar->maximum());

    // TODO support negative npages
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

void TerminalWidget::doVerticalTab()
{
    onShellRead(QString("\n\n\n").toUtf8());
}

