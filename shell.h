#ifndef SHELL_H
#define SHELL_H

#include <QObject>
#include <QProcess>

/** Communicates with the shell subprocess
  *
  * TODO
  *
  * This needs to be replaced by an abstract shell driver.
  * On unices, the driver allocates a ptty and uses stdin/out/err
  * On windows, cmd.exe works very poorly with std streams.
  * Comment about console.sf.net mentions the hacky windows workaround:
  *    http://stackoverflow.com/questions/8850461/terminal-emulator-implementation-problems-with-repeated-input
  */
class Shell : public QObject
{
    Q_OBJECT

public:
    Shell(const QString &command, const QStringList &args = QStringList());
    virtual ~Shell();

    const QString &command() const;

    const QStringList &args() const;

    void open();
    void close();

    bool isOpen();

    bool write(char c);
    size_t write(char *buf, size_t len);
    QString write(const QString &str);

signals:
    void read(const QByteArray &);
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
