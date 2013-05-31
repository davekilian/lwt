#ifndef CURSOR_H
#define CURSOR_H

#include <QPainter>

class TerminalWidget;

/** Represents the user's input cursor.
 *
 *  Tracks a cursor's location, renders it, and manages blinking logic.
 */
class Cursor 
{
public:
    Cursor(TerminalWidget *m_parent);
    ~Cursor();

    /** Gets this cursor's current row (i.e. line number) */
    int row() const;

    /** Gets this cursor's current column (i.e. position within the line) */
    int col() const;

    /** Moves this cursor to the given position */
    void moveTo(int row, int col);

    /** Moves this cursor by the given amount */
    void moveBy(int rowDelta, int colDelta);

    /** Renders this cursor
     *
     *  If the cursor is currently in the 'off' stage of the blink cycle, this
     *  call does nothing. Otherwise, the cursor draws itself and redraws the
     *  character under the cursor as necessary.
     *
     *  @param painter  The painter to use to draw the cursor
     */
    void render(QPainter &painter);

private:
    TerminalWidget *m_parent;

    int m_row;
    int m_col;
};

// TODO blinking logic
//      include a QTimer and have some kind of parent notify event

#endif // CURSOR_H
