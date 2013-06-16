#ifndef SHELL_H
#define SHELL_H

#include <QObject>

/** Abstract base for a shell driver.
 *  
 *  Shell drivers manage interaction with the shell binary. All user input is
 *  funneled through the shell via the driver, and the driver produces shell
 *  output which is rendered to the screen via the TerminalWidget.
 */
class Shell : public QObject
{
    Q_OBJECT

public:
    /** Instantiates the proper shell driver for the current platform.
     *
     *  If there is no platform-specific driver for the platform this
     *  application is being compiled for, create() returns a new ProcessShell
     *  as a fallback.
     */
    static Shell* create();

    /** Opens this shell. If necessary, a shell binary will be spawned.
     *
     *  This action can cause the shell to produce output. Thus you should
     *  connect to this shell's read and closed signals before you call open()
     */
    virtual void open() = 0;

    /** Ends interaction with the shell binary, and kills the shell binary if
     *  necessary. 
     *
     *  This action can cause the shell to produce output. Thus you should not
     *  disconnect from the shell's read and closed signals until after you've
     *  called close().
     */
    virtual void close() = 0;

    /** Indicates whether the shell binary is running and has not exited */
    virtual bool isOpen() = 0;

    /** Writes the given data to the shell's stdin stream. 
     *
     *  The shell driver is allowed to convert the string data to a different
     *  character encoding as necessary. For the most part, shell drivers will
     *  convert to ASCII or UTF-8.
     *
     *  If the shell cannot accept all of the given input buffer in one call,
     *  this method returns the data that could not be written. If no data
     *  could be written, this method returns the entire input buffer. This
     *  call is not allowed to block until the shell is ready to receive all
     *  bytes. It must be safe to call this method on the thread that runs the
     *  application's event queue without losing responsiveness.
     *
     *  @param buf  The data to write
     *  @return     The data that could not be written
     */
    virtual QString write(const QString &buf) = 0;

signals:
    /** Emitted whenever the shell writes to stdout or stderr
     *  
     *  @param input The data written to stdout or stderr
     */
    void read(const QString &data);

    /** Emitted when the shell binary closes */
    void closed();
};

#endif // SHELL_H
