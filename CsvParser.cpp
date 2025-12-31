#include "CsvParser.h"
#include "Logger.h"
#include <QFile>
#include <QTextStream>

bool CsvParser::parseFile(const QString &filePath, CsvStats &stats)
{
    stats = CsvStats{};

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::log(Logger::Level::Error,
                    QString("Failed to open CSV: %1 (%2)")
                        .arg(filePath, file.errorString()));
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QStringList parts;
        if (!parseCsvLine(in, parts)) {
            Logger::log(Logger::Level::Warn,
                        QString("Malformed CSV line in %1").arg(filePath));
            continue;
        }

        if (parts.isEmpty())
            continue;

        stats.rowCount++;

        bool ok = false;
        double v = parts[0].toDouble(&ok);
        if (ok) {
            stats.numericCount++;
            stats.firstColumnSum += v;
        }
    }

    Logger::log(Logger::Level::Debug,
                QString("Parsed %1: rows=%2 numeric=%3 sum=%4")
                    .arg(filePath)
                    .arg(stats.rowCount)
                    .arg(stats.numericCount)
                    .arg(stats.firstColumnSum));

    return true;
}


bool CsvParser::parseCsvLine(QTextStream& ts, QStringList& row)
{
    QString field;
    bool inQuotes = false;

    while(!ts.atEnd()){
        QString oneLine = ts.readLine();
        for (int i = 0; i < oneLine.size(); ++i) {
            QChar c = oneLine[i];

            if (inQuotes) {
                if (c == '"') {
                    // 次が " ならエスケープ
                    if (i + 1 < oneLine.size() && oneLine[i + 1] == '"') {
                        field += '"';
                        i++; // 次の " をスキップ
                    } else {
                        inQuotes = false;
                    }
                } else {
                    field += c;
                }
            } else {
                if (c == '"') {
                    inQuotes = true;
                } else if (c == ',') {
                    row << field;
                    field.clear();
                } else {
                    field += c;
                }
            }
        }

        // 「"」の範囲が終わっていなければ続行
        if(!inQuotes)
            break;

        field += '\n';  // 続行する前に改行を挿入
    }

    row << field;

    // 「"」が閉じているかを判定
    return !inQuotes;
}