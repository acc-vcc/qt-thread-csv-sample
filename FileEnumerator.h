#ifndef FILEENUMERATOR_H
#define FILEENUMERATOR_H

#include <QString>
#include <QVector>

class FileEnumerator
{
public:
    static QVector<QString> enumerateCsvFiles(const QString &directoryPath);
};

#endif // FILEENUMERATOR_H
