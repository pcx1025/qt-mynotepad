#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    // ---- 自动化测试/截图辅助参数（不影响正常使用）----
    const QStringList args = a.arguments();
    if (args.contains("--sample"))  w.loadSampleText();
    if (args.contains("--styled"))  w.applyDemoStyle();
    if (args.contains("--modified")) w.setDemoModified();
    if (args.contains("--hidetb"))  w.setDemoToolbarVisible(false);
    if (args.contains("--hidestatus")) w.setDemoStatusbarVisible(false);
    if (args.contains("--nolines")) w.setDemoLineNumbers(false);
    if (args.contains("--wrap"))    w.setDemoWrap(true);
    if (args.contains("--select"))  w.selectDemoText();
    for (const QString &arg : args) {
        if (arg.startsWith("--show="))
            w.demoShowDialog(arg.mid(7));   // about / find / replace
    }

    return a.exec();
}
