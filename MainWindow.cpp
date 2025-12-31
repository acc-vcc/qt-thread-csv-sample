#include "MainWindow.h"
#include "Worker.h"
#include "Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QTextBrowser>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("CSV一括解析ツール");
    setupUi();
    connectSignals();
    updateUiState();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // 入力フォルダ
    {
        auto *layout = new QHBoxLayout;
        layout->addWidget(new QLabel("入力フォルダ:", this));

        m_inputSelectButton = new QPushButton("選択...", this);
        m_inputDirEdit = new QLineEdit(this);
        m_inputDirEdit->setPlaceholderText("未選択");

        layout->addWidget(m_inputSelectButton);
        layout->addWidget(m_inputDirEdit);

        mainLayout->addLayout(layout);
    }

    // 出力フォルダ
    {
        auto *layout = new QHBoxLayout;
        layout->addWidget(new QLabel("出力フォルダ:", this));

        m_outputSelectButton = new QPushButton("選択...", this);
        m_outputDirEdit = new QLineEdit("./out", this);

        layout->addWidget(m_outputSelectButton);
        layout->addWidget(m_outputDirEdit);

        mainLayout->addLayout(layout);
    }

    // 開始 / 中止
    m_startStopButton = new QPushButton("開始", this);
    mainLayout->addWidget(m_startStopButton);

    // 進捗バー
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    mainLayout->addWidget(m_progressBar);

    // ログ
    m_logView = new QTextBrowser(this);
    mainLayout->addWidget(m_logView, 1);
}

void MainWindow::connectSignals()
{
    connect(m_inputSelectButton, &QPushButton::clicked,
            this, &MainWindow::onSelectInputDir);

    connect(m_outputSelectButton, &QPushButton::clicked,
            this, &MainWindow::onSelectOutputDir);

    connect(m_startStopButton, &QPushButton::clicked,
            this, &MainWindow::onStartStopClicked);

    connect(&Logger::instance(), &Logger::newLog,
            this, &MainWindow::onNewLog);
}

void MainWindow::updateUiState()
{
    bool running = (m_state == State::Running);

    m_inputSelectButton->setEnabled(!running);
    m_inputDirEdit->setEnabled(!running);

    m_outputSelectButton->setEnabled(!running);
    m_outputDirEdit->setEnabled(!running);

    m_progressBar->setEnabled(running);

    m_startStopButton->setText(running ? "中止" : "開始");

    if (!running)
        m_progressBar->setValue(0);
}

void MainWindow::onSelectInputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "入力フォルダを選択");
    if (!dir.isEmpty()) {
        m_inputDirEdit->setText(dir);
        Logger::log(Logger::Level::Info,
                    QString("Input directory selected: %1").arg(dir));
    } else {
        Logger::log(Logger::Level::Debug,
                    "Input directory selection canceled.");
    }
}

void MainWindow::onSelectOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "出力フォルダを選択");
    if (!dir.isEmpty()) {
        m_outputDirEdit->setText(dir);
        Logger::log(Logger::Level::Info,
                    QString("Output directory selected: %1").arg(dir));
    } else {
        Logger::log(Logger::Level::Debug,
                    "Output directory selection canceled.");
    }
}

void MainWindow::onStartStopClicked()
{
    if (m_state == State::Idle)
        startWorker();
    else
        stopWorkerRequest();
}

void MainWindow::startWorker()
{
    if (m_state == State::Running) {
        Logger::log(Logger::Level::Debug,
                    "Start requested but worker is already running.");
        return;
    }

    QString inputDir = m_inputDirEdit->text().trimmed();
    QString outputDir = m_outputDirEdit->text().trimmed();
    if (outputDir.isEmpty())
        outputDir = "./out";

    if (inputDir.isEmpty()) {
        Logger::log(Logger::Level::Warn,
                    "Start requested but input directory is empty.");
        QMessageBox::warning(this, "警告", "入力フォルダが選択されていません。");
        return;
    }

    m_logView->clear();

    Logger::log(Logger::Level::Info,
                QString("Processing started. inputDir=%1, outputDir=%2")
                    .arg(inputDir, outputDir));

    if (m_worker) {
        m_worker->disconnect();
        m_worker->deleteLater();
        m_worker = nullptr;
    }

    m_worker = new Worker(this);
    m_worker->setInputDirectory(inputDir);
    m_worker->setOutputDirectory(outputDir);

    connect(m_worker, &Worker::progress,
            this, &MainWindow::onWorkerProgress);
    connect(m_worker, &Worker::finishedSuccessfully,
            this, &MainWindow::onWorkerFinished);
    connect(m_worker, &Worker::canceled,
            this, &MainWindow::onWorkerCanceled);
    connect(m_worker, &Worker::failed,
            this, &MainWindow::onWorkerFailed);
    connect(m_worker, &QThread::finished, this, [this] {
        if (m_state == State::Running) {
            m_state = State::Idle;
            updateUiState();
        }
    });
    m_state = State::Running;
    updateUiState();

    m_worker->start();
}

void MainWindow::stopWorkerRequest()
{
    if (!m_worker || !m_worker->isRunning()) {
        Logger::log(Logger::Level::Warn,
                    "Cancel requested but worker is not running.");
        return;
    }

    auto ret = QMessageBox::question(
                this, "確認", "処理を中止しますか？",
                QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        Logger::log(Logger::Level::Info, "User requested cancel.");
        m_worker->requestCancel();
    } else {
        Logger::log(Logger::Level::Debug, "Cancel request dialog dismissed.");
    }
}

void MainWindow::onWorkerProgress(int percent)
{
    m_progressBar->setValue(percent);
}

void MainWindow::onWorkerFinished()
{
    Logger::log(Logger::Level::Success, "Worker finished successfully.");

    m_state = State::Idle;
    updateUiState();

    QMessageBox::information(this, "完了", "処理が完了しました。");
}

void MainWindow::onWorkerCanceled()
{
    Logger::log(Logger::Level::Info, "Worker canceled.");

    m_state = State::Idle;
    updateUiState();

    QMessageBox::information(this, "中止", "処理が中止されました。");
}

void MainWindow::onWorkerFailed(const QString &message)
{
    Logger::log(Logger::Level::Info,
                QString("Worker failed: %1").arg(message));

    m_state = State::Idle;
    updateUiState();

    QMessageBox::critical(this, "エラー", message);
}

void MainWindow::onNewLog(const QString &htmlLine)
{
    m_logView->append(htmlLine);
}
