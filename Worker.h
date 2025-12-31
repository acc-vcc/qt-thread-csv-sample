#ifndef WORKER_H
#define WORKER_H

#include <QThread>
#include <QString>
#include <atomic>

class Worker : public QThread
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker() override;

    void setInputDirectory(const QString &dir);
    void setOutputDirectory(const QString &dir);

    void requestCancel();

signals:
    void progress(int percent);
    void finishedSuccessfully();
    void canceled();
    void failed(const QString &message);

protected:
    void run() override;

private:
    std::atomic_bool m_cancelRequested;
    QString m_inputDir;
    QString m_outputDir;
};

#endif // WORKER_H
