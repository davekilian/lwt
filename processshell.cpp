#include "processshell.h"

ProcessShell::ProcessShell(const QString &command, const QStringList &args)
    : m_command(command),
      m_args(args)
{ }

ProcessShell::~ProcessShell()
{
    if (m_process.isOpen())
        m_process.close();
}

const QString &ProcessShell::command() const
{
    return m_command;
}

const QStringList &ProcessShell::args() const
{
    return m_args;
}

void ProcessShell::open()
{
    if (isOpen())
        close();

    connect(&m_process, SIGNAL(readyReadStandardOutput()), SLOT(onstdout()));
    connect(&m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(onclose()));

    m_process.setReadChannel(QProcess::StandardOutput);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    m_process.start(m_command, m_args);
}

void ProcessShell::close()
{
    m_process.close();
}

bool ProcessShell::isOpen()
{
    return m_process.isOpen();
}

QString ProcessShell::write(const QString &str)
{
    QByteArray ba = str.toUtf8();

    size_t nw = m_process.write(ba);
    ba.remove(0, nw);

    return QString(ba);
}

void ProcessShell::onstdout()
{
    while (m_process.bytesAvailable() > 0)
    {
        QByteArray bytes = m_process.read(m_process.bytesAvailable());
        emit read(bytes);
    }
}

void ProcessShell::onclose()
{
    emit closed();
}

