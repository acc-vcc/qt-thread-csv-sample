#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum class Level {
        Debug,
        Info,
        Success,
        Warn,
        Error
    };
    Q_ENUM(Level)

    static Logger &instance();

    static void log(Level level, const QString &message);

    void setLogFilePath(const QString &filePath);

signals:
    void newLog(const QString &htmlLine);

private:
    explicit Logger(QObject *parent = nullptr);

    static QString timestamp();
    static QString levelToString(Level level);
    static QString colorForLevel(Level level);
    static QString formatHtml(Level level, const QString &message);

    void writeToFile(const QString &plainLine);

private:
    QFile m_logFile;
    QTextStream m_stream;
    QMutex m_mutex;
};

#endif // LOGGER_H
