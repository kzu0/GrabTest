#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QPaintEvent>
#include <QImage>

//-------------------------------------------------------------------------------------------------
PaintEventFilter::PaintEventFilter(QObject *parent, QWidget* sourceWidget) : QObject(parent), sourceWidget(sourceWidget) {

}

//-------------------------------------------------------------------------------------------------
bool PaintEventFilter::eventFilter(QObject* obj, QEvent* event) {

    if (event->type() == QEvent::Paint) {
        if (sourceWidget != nullptr) {
            emit painted(sourceWidget->grab().toImage().convertToFormat(QImage::Format_RGB888));
        }
        return true;

    } else {

        // Standard event processing
        return QObject::eventFilter(obj, event);
    }
}

//-------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), paintEventFilter(nullptr), gstDisplay(nullptr) {

    ui->setupUi(this);
    ui->renderLabel->setScaledContents(true);

    gstDisplay = new GstDisplay(this);

    paintEventFilter = new PaintEventFilter(this, ui->sourceWidget);

    connect(paintEventFilter, SIGNAL(painted(QImage)), this, SLOT(OnPainted(QImage)));
    connect(paintEventFilter, SIGNAL(painted(QImage)), gstDisplay, SLOT(OnPainted(QImage)));

    ui->sourceFrame->installEventFilter(paintEventFilter);
}

//-------------------------------------------------------------------------------------------------
MainWindow::~MainWindow() {

    delete ui;
}

//-------------------------------------------------------------------------------------------------
void MainWindow::OnPainted(QImage image) {

    ui->renderLabel->setPixmap(QPixmap::fromImage(image));
}

