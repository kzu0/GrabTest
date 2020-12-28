#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QPaintEvent>

//-------------------------------------------------------------------------------------------------
PaintEventFilter::PaintEventFilter(QObject *parent, QWidget* sourceWidget) : QObject(parent), sourceWidget(sourceWidget) {

}

//-------------------------------------------------------------------------------------------------
bool PaintEventFilter::eventFilter(QObject* obj, QEvent* event) {

    if (event->type() == QEvent::Paint) {
        if (sourceWidget != nullptr) {
            emit painted(sourceWidget->grab().toImage());
        }
        return true;

    } else {

        // Standard event processing
        return QObject::eventFilter(obj, event);
    }
}

//-------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);
    ui->renderLabel->setScaledContents(true);

    paintEventFilter = nullptr;
    paintEventFilter = new PaintEventFilter(this, ui->sourceWidget);
    connect(paintEventFilter, SIGNAL(painted(QImage)), this, SLOT(OnPainted(QImage)));

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

