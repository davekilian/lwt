#include "shell.h"

Shell::Shell(const QString &command, const QStringList &args)
    : QObject(NULL),
      m_command(command),
      m_args(args)
{ }

Shell::~Shell()
{
    if (m_process.isOpen())
        m_process.close();
}

const QString &Shell::command() const
{
    return m_command;
}

const QStringList &Shell::args() const
{
    return m_args;
}

void Shell::open()
{
    if (isOpen())
        close();

    connect(&m_process, SIGNAL(readyReadStandardOutput()), SLOT(onstdout()));
    connect(&m_process, SIGNAL(readyReadStandardError()), SLOT(onstderr()));
    connect(&m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(onclose()));

    m_process.setReadChannel(QProcess::StandardOutput);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    m_process.start(m_command, m_args);
}

void Shell::close()
{
    m_process.close();
}

bool Shell::isOpen()
{
    return m_process.isOpen();
}

bool Shell::write(char c)
{
    return m_process.write(&c, 1) == 1;
}

size_t Shell::write(char *buf, size_t len)
{
    return m_process.write(buf, len);
}

QString Shell::write(const QString &str)
{
    size_t nw = m_process.write(str.toUtf8());
    return str.right(str.length() - nw);
}

void Shell::onstdout()
{
    read();
}

void Shell::read()
{
    while (m_process.bytesAvailable() > 0)
    {
        QByteArray bytes = m_process.read(m_process.bytesAvailable());
        emit read(bytes);
    }
}

void Shell::onclose()
{
    emit closed();
}
