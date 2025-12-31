#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QString>

struct CsvStats {
    int rowCount = 0;
    int numericCount = 0;
    double firstColumnSum = 0.0;
};

class QTextStream;

class CsvParser
{
public:
    static bool parseFile(const QString &filePath, CsvStats &stats);

private:
    static bool parseCsvLine(QTextStream& ts, QStringList& row);
};

#endif // CSVPARSER_H
