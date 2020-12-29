#include "MainWindow.h"

#include <QApplication>
#include <QDebug>

#include "gst/gst.h"

int main(int argc, char *argv[]) {


    // Initialize the GStreamer library
    if (gst_init_check(&argc, &argv, NULL)) {
        qInfo() << "This program is linked against" << gst_version_string();
    }



    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
