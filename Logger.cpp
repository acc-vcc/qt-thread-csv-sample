#include "Logger.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>

Logger &Logger::instance()
{
    static Logger inst;
    return inst;
}

Logger::Logger(QObject *parent)
    : QObject(parent),
      m_stream(&m_logFile)
{
}

void Logger::setLogFilePath(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen())
        m_logFile.close();

    m_logFile.setFileName(filePath);

    QDir dir = QFileInfo(m_logFile).dir();
    dir.mkpath(".");

    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Logger: failed to open log file:" << filePath;
        return;
    }

    // QTextStream を作り直す
    m_stream.flush();
    m_stream.setDevice(&m_logFile);
    m_stream.setCodec("UTF-8");
}

void Logger::log(Level level, const QString &message)
{
#ifdef NO_DEBUG_LOG
    if (level == Level::Debug) return;
#endif

    Logger &inst = instance();

    QString plain = QString("[%1][%2] %3")
            .arg(timestamp())
            .arg(levelToString(level))
            .arg(message);

    QString html = formatHtml(level, plain);

    {
        QMutexLocker locker(&inst.m_mutex);
        inst.writeToFile(plain);
    }

    emit inst.newLog(html);
}

QString Logger::timestamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

QString Logger::levelToString(Level level)
{
    switch (level) {
    case Level::Debug:   return "DEBUG";
    case Level::Info:    return "INFO";
    case Level::Success: return "SUCCESS";
    case Level::Warn:    return "WARN";
    case Level::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

QString Logger::colorForLevel(Level level)
{
    switch (level) {
    case Level::Debug:   return "#808080";
    case Level::Info:    return "#000000";
    case Level::Success: return "#008000";
    case Level::Warn:    return "#CC6600";
    case Level::Error:   return "#CC0000";
    }
    return "#000000";
}

QString Logger::formatHtml(Level level, const QString &message)
{
    return QString("<span style=\"color:%1;\">%2</span>")
            .arg(colorForLevel(level))
            .arg(message.toHtmlEscaped());
}

void Logger::writeToFile(const QString &line)
{
    if (!m_logFile.isOpen())
        return;

    m_stream << line << "\n";
    m_stream.flush();
}
