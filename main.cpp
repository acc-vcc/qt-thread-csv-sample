#include <QApplication>
#include "MainWindow.h"
#include "Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ログ出力先は固定
    Logger::instance().setLogFilePath("./log.txt");
    Logger::log(Logger::Level::Info, "Application starting.");

    MainWindow w;
    w.resize(400, 300);
    w.show();

    Logger::log(Logger::Level::Info, "Main window shown.");

    return app.exec();
}
