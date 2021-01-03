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

    switch(event->type()) {

    case QEvent::Paint:
        if (sourceWidget != nullptr) {
            emit painted(sourceWidget->grab().toImage().convertToFormat(QImage::Format_RGB888));
        }
        return true;
        break;

    case QEvent::Resize:
        if (sourceWidget != nullptr) {
            emit resized(QSize(sourceWidget->width(), sourceWidget->height()));
        }
        return true;
        break;

    default:
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
    connect(paintEventFilter, SIGNAL(resized(QSize)), gstDisplay, SLOT(OnResized(QSize)));

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

