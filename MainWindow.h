#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include "Logger.h"

class QLineEdit;
class QPushButton;
class QProgressBar;
class QTextBrowser;
class Worker;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSelectInputDir();
    void onSelectOutputDir();
    void onStartStopClicked();

    void onWorkerProgress(int percent);
    void onWorkerFinished();
    void onWorkerCanceled();
    void onWorkerFailed(const QString &message);

    void onNewLog(const QString &htmlLine);

private:
    enum class State { Idle, Running };

    void setupUi();
    void connectSignals();
    void updateUiState();
    void startWorker();
    void stopWorkerRequest();

private:
    QLineEdit *m_inputDirEdit = nullptr;
    QPushButton *m_inputSelectButton = nullptr;

    QLineEdit *m_outputDirEdit = nullptr;
    QPushButton *m_outputSelectButton = nullptr;

    QPushButton *m_startStopButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QTextBrowser *m_logView = nullptr;

    QPointer<Worker> m_worker;
    State m_state = State::Idle;
};

#endif // MAINWINDOW_H
