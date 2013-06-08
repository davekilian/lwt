#ifndef CURSOR_H
#define CURSOR_H

#include <QPainter>
#include <QTimer>

class TerminalWidget;

/** Represents the user's input cursor.
 *
 *  Tracks a cursor's location, renders it, and manages blinking logic.
 */
class Cursor : public QObject
{
    Q_OBJECT

public:
    Cursor(TerminalWidget *m_parent);
    ~Cursor();

    /** Gets this cursor's current row (i.e. line number) */
    int row() const;

    /** Gets this cursor's current column (i.e. position within the line) */
    int col() const;

    /** Hides this cursor.
     *
     *  If a time is given, the cursor is shown after the given number of
     *  milliseconds has elapsed.
     */
    void hide(int ms = 500);

    /** Shows the cursor, cancelling a previous hide() operation */
    void show();

    /** Gets or sets the amount of time the cursor is shown when blinking,
     *  in milliseconds.
     */
    int blinkOn() const;
    void setBlinkOn(int);

    /** Gets or sets the amount of time the cursor is hidden when blinking, in
      * milliseconds.
      */
    int blinkOff() const;
    void setBlinkOff(int);

    /** Gets or sets the amount of time the cursor remains visible after it is
     *  moved, in milliseconds
     */
    int blinkPause() const;
    void setBlinkPause(int);

    /** Renders this cursor
     *
     *  If the cursor is currently in the 'off' stage of the blink cycle, this
     *  call does nothing. Otherwise, the cursor draws itself and redraws the
     *  character under the cursor as necessary.
     *
     *  @param painter  The painter to use to draw the cursor
     */
    void render(QPainter &painter);

public slots:
    /** Moves this cursor to the given position */
    void moveTo(int row, int col);

    /** Moves this cursor by the given amount */
    void moveBy(int rowDelta, int colDelta);

private slots:
    void onBlinkTimer();
    void onHideTimer();

private:
    TerminalWidget *m_parent;

    int m_row;
    int m_col;

    bool m_hidden;
    QTimer m_hideTimer;

    bool m_blinkVisible;
    int m_blinkOn;
    int m_blinkOff;
    int m_blinkPause;

    QTimer m_blinkTimer;

    void beginOnBlink();
    void beginOffBlink();
    void beginPauseBlink();
};

#endif // CURSOR_H
