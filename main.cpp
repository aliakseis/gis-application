#include <QApplication>
#include <QStyleFactory>

#include "mainwidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

#ifdef Q_OS_LINUX
    // Need for correct work std::atof function on Linux
    // https://doc.qt.io/qt-5/qcoreapplication.html#locale-settings
    // On Unix/Linux Qt is configured to use the system locale settings by
    // default. This can cause a conflict when using POSIX functions, for
    // instance, when converting between data types such as floats and strings,
    // since the notation may differ between locales. To get around this
    // problem, call the POSIX function setlocale(LC_NUMERIC,"C") right after
    // initializing QApplication, QGuiApplication or QCoreApplication to reset
    // the locale that is used for number formatting to "C"-locale.
    std::setlocale(LC_NUMERIC, "C");
#endif

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainWidget w;
    w.show();

    return QApplication::exec();
}
