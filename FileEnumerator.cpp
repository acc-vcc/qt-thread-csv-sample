#include "FileEnumerator.h"
#include "Logger.h"

#include <QDir>
#include <QFileInfoList>

QVector<QString> FileEnumerator::enumerateCsvFiles(const QString &directoryPath)
{
    QVector<QString> result;

    QDir dir(directoryPath);
    if (!dir.exists()) {
        Logger::log(Logger::Level::Warn,
                    QString("Directory does not exist: %1").arg(directoryPath));
        return result;
    }

    QFileInfoList list = dir.entryInfoList(
        QStringList() << "*.csv" << "*.CSV",
        QDir::Files | QDir::NoSymLinks);

    result.reserve(list.size());
    for (const QFileInfo &info : list)
        result.append(info.absoluteFilePath());

    return result;
}
