#ifndef SHELL_H
#define SHELL_H

#include <QObject>
#include <QProcess>

/** Manages interaction with a shell binary.
 *
 *  This is a prototype version, which just spins up a shell process and
 *  communicates via stdin / stdout / stderr. In the future, this object will
 *  be replaced by an abstract shell driver, since different behavior will be
 *  needed on different platforms.
 */
class Shell : public QObject
{
    Q_OBJECT

public:
    Shell(const QString &command, const QStringList &args = QStringList());
    virtual ~Shell();

    /** The shell binary to run when the shell is opened */
    const QString &command() const;

    /** The list of arguments to pass to the shell binary */
    const QStringList &args() const;

    /** Spawns the shell process and begins I/O.
     *
     *  This action can cause the shell to produce output. Thus you should
     *  connect to this shell's read and closed signals before you call open().
     */
    void open();

    /** Ends interaction with the shell binary and kills the process */
    void close();

    /** Indicates whether the shell binary is running and has not exited */
    bool isOpen();

    /** Writes a single character to the shell process's stdin stream.
     *
     *  @return     true iff the write succeeded
     */
    bool write(char c);

    /** Writes data to the shell process's stdin stream.
     *
     *  @param buf  A buffer containing the byte stream to write
     *  @param len  The number of bytes to write
     *  @return     The number of bytes written
     */
    size_t write(char *buf, size_t len);

    /** Converts the given string's contents to a UTF-8 byte stream (including
     *  the null terminator) and writes those bytes to the shell process's
     *  stdin stream.
     *
     *  @param str  The string containing the characters to write.
     */
    QString write(const QString &str);

signals:
    /** Triggered whenever the shell sends data to its stdout or stderr
     *
     *  @param data Contains the data that was written
     */
    void read(const QByteArray &data);

    /** Triggered when the shell process exits */
    void closed();

private slots:
    void onstdout();
    void onstderr();
    void onclose();

private:
    QString m_command;
    QStringList m_args;
    QProcess m_process;

    void read();
};

#endif // SHELL_H
