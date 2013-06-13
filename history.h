#ifndef HISTORY_H
#define HISTORY_H

#include "specialchars.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

/** Manages writing to and querying from scrollback history.
 *  This object also receives and processes most escape sequences. 
 */
class History : public QObject
{
    Q_OBJECT

public:
    History();
    ~History();

    /** Sets up this history object to receive escape sequence signals from the
     *  given escape-sequence handler
     */
    void connectTo(SpecialChars *chars) const;

    /** Must be called before you begin write()ing characters */
    void beginWrite();

    /** Writes the given character into the history buffer at the cursor */
    void write(QChar c);

    /** Must be called after you finish write()ing characters */
    void endWrite();

    /** Gets the character at the given row and column. Returns the space
     *  character (' ') if there is no character in the given cell. This method
     *  takes word wrap into account.
     *
     *  The Cursor object uses this to draw the character under the cursor
     */
    QChar charAt(int row, int col) const;

    /** Gets the $(index)th line of text on the screen, after taking word wrap
     *  into account.
     */
    QString line(int index) const;

    /** Returns a list of lines that are currently visible, after taking word
     *  wrap into account
     *
     *  @param yTop         The Y coordinate in pixels of the top of the 
     *                      viewport
     *  @param yBottom      The Y coordinate in pixels of the bottom of 
     *                      the viewport
     *  @param lineHeight   The distance in pixels between consecutive lines
     *                      when rendered to the screen
     */
    QStringList visibleLines(int yTop, int yBottom, int lineHeight) const;

    /** Returns the number of lines of text this history contains */
    int numLines() const;

    /** Must be called when the viewport is resized horizontally.
     *
     *  This method performs a word-wrap on the original data received from the
     *  shell to fit the new width of the viewport.
     *
     *  This method also caches the number of rows and columns visible, which
     *  is needed in order to process some escape sequences (e.g. scrolling)
     */
    void onViewportResized(int numRowsVisible, int numColsVisible);
    
signals:
    /** Raised whenever an input event or escape sequence causes the cursor to
     *  move
     *
     *  @param row  Receives the new (zero-indexed) row of the cursor
     *  @param col  Receives the new (zero-indexed) column of the cursor
     */
    void cursorMoved(int row, int col);

    /** Raised whenever the contents of the history buffer changes.
     *  This would be a good time to e.g. redraw the viewport
     */
    void updated();

    /** Raised whenever this history object thinks the terminal view should be
     *  scrolled to the bottom. This happens e.g. when processing an ASCII
     *  form-feed (FF) character
     */
    void scrollToBottom();

private slots:
    /** Slots activated by a SpecialChars object specified in a connectTo()
     *  call. See specialchars.h for details about the individual signals.
     *
     *  Note: these handlers are all implemented assuming they were called
     *  inside a beginWrite / endWrite block.
     */

    void carriageReturn();
    void del(int n);
    void erase(SpecialChars::EraseType type);
    void formFeed();
    void horizontalTab();
    void insert(int n);
    void moveCursorBy(int rowDelta, int colDelta);
    void moveCursorTo(int row, int col);
    void resetColors();
    void setColor(SpecialChars::Color c, bool bright, bool foreground);
    void setColor256(int index, bool foreground);
    void verticalTab();

private:
    /** Description of a "virtual line" or vline. 
     *
     *  Each line of text received from the shell corresponds to one or more
     *  vlines, which contain that text word-wrapped for the current viewport
     *  size. 
     *
     *  The History object contains a list of (unchanged, original) lines 
     *  received from the shell, as well as a list of vlines, which is thrown 
     *  away and recomputed whenever the viewport size changes.
     */
    struct vline
    {
        vline() : line(0), beg(0), len(0) { }
        vline(int line, int beg, int len) : line(line), beg(beg), len(len) { }

        /** The canonical line number this vline corresponds to.
         *  This is NOT the virtual line number -- instead, use the index of
         *  this vline into m_vlines
         */
        int line;

        /** The index into m_lines[this->line] that this vline begins */
        int beg;

        /** The length of this virtual line */
        int len;
    };

    /** Description of a graphics event */
    struct gevent
    {
        /** The canonical line number this event occured on
         *  This indexes into m_lines, not m_vlines
         */
        int line;

        /** The index into m_lines[this->line] this event occurred */
        int col;

        /** True if this event modified the foreground color;
         *  false if this event modified the background color
         */
        bool foreground;

        /** The color palette index of this event, using the xterm-256 color
         *  palette scheme
         */
        int color;
    };

    /** The list of canonical lines
     *
     *  The item at the i'th index of this list is the i'th line of text, as
     *  received from the shell. This contains the list of lines as they would
     *  be rendered if the viewport were infinitely large.
     */
    QVector<QString>    m_lines;

    /** The list of word-wrapped lines.
     *  See the description of the vline struct
     */
    QVector<vline>      m_vlines;

    /** The list of graphics events
     *  See the description of the gevent struct
     */
    QVector<gevent>     m_gevents;

    /** The virtual line number of the user's cursor */
    int                 m_cursorLine;

    /** The index into the virtual line of this user's cursor */
    int                 m_cursorCol;

    /** The height of the viewport in rows */
    int                 m_numRowsVisible;

    /** The width of the viewport in columns */
    int                 m_numColsVisible;

    /** Recomputes m_vlines based on m_lines and m_numVisibleCols */
    void wrapLines();
};

#endif
