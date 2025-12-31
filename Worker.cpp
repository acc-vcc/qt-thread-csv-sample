#include "Worker.h"
#include "FileEnumerator.h"
#include "CsvParser.h"
#include "Logger.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

Worker::Worker(QObject *parent)
    : QThread(parent),
      m_cancelRequested(false)
{
}

Worker::~Worker()
{
    requestCancel();
    wait();
}

void Worker::setInputDirectory(const QString &dir)
{
    m_inputDir = dir;
}

void Worker::setOutputDirectory(const QString &dir)
{
    m_outputDir = dir;
}

void Worker::requestCancel()
{
    m_cancelRequested.store(true);
}

void Worker::run()
{
    m_cancelRequested.store(false);

    if (m_inputDir.isEmpty()) {
        QString msg = "Input directory is empty.";
        Logger::log(Logger::Level::Error, msg);
        emit failed(msg);
        return;
    }

    auto files = FileEnumerator::enumerateCsvFiles(m_inputDir);
    if (files.isEmpty()) {
        QString msg = "No CSV files found in input directory.";
        Logger::log(Logger::Level::Warn, msg);
        emit failed(msg);
        return;
    }

    QDir outDir(m_outputDir.isEmpty() ? "./out" : m_outputDir);
    if (!outDir.mkpath(".")) {
        QString msg = QString("Failed to create output directory: %1")
                        .arg(outDir.absolutePath());
        Logger::log(Logger::Level::Error, msg);
        emit failed(msg);
        return;
    }

    QString summaryPath = outDir.absoluteFilePath("summary.txt");
    QFile summaryFile(summaryPath);
    if (!summaryFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QString msg = QString("Failed to open summary file: %1 (%2)")
                        .arg(summaryPath, summaryFile.errorString());
        Logger::log(Logger::Level::Error, msg);
        emit failed(msg);
        return;
    }

    QTextStream out(&summaryFile);

    int total = files.size();
    int processed = 0;

    int totalRows = 0;
    int totalNumeric = 0;
    double totalSum = 0.0;

    Logger::log(
        Logger::Level::Info,
        QString("Start processing %1 CSV files.").arg(total)
    );

    for (const QString &file : files) {

        if (m_cancelRequested.load()) {
            Logger::log(Logger::Level::Warn, "Processing canceled by user.");
            summaryFile.close();
            summaryFile.remove();
            emit canceled();
            return;
        }

        Logger::log(Logger::Level::Info,
                    QString("Processing file: %1").arg(file));

        CsvStats stats;
        bool ok = CsvParser::parseFile(file, stats);

        if (ok) {
            out << "File: " << file << "\n";
            out << "  Rows: " << stats.rowCount << "\n";
            out << "  Numeric count: " << stats.numericCount << "\n";
            out << "  Sum: " << stats.firstColumnSum << "\n\n";

            totalRows += stats.rowCount;
            totalNumeric += stats.numericCount;
            totalSum += stats.firstColumnSum;
        } else {
            Logger::log(Logger::Level::Error,
                        QString("Failed to process file: %1").arg(file));
        }

        processed++;
        emit progress((processed * 100) / total);
    }

    out << "=== Summary ===\n";
    out << "Total files: " << total << "\n";
    out << "Total rows: " << totalRows << "\n";
    out << "Total numeric: " << totalNumeric << "\n";
    out << "Total sum: " << totalSum << "\n";

    Logger::log(
        Logger::Level::Success,
        QString("Finished. files=%1 rows=%2 numeric=%3 sum=%4")
            .arg(total)
            .arg(totalRows)
            .arg(totalNumeric)
            .arg(totalSum)
    );

    emit finishedSuccessfully();
}
