#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GstDisplay.h"

//-------------------------------------------------------------------------------------------------
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//-------------------------------------------------------------------------------------------------
class PaintEventFilter : public QObject {

    Q_OBJECT

public:
    PaintEventFilter(QObject* parent = nullptr, QWidget* sourceWidget = nullptr);

private:
    QWidget* sourceWidget;

signals:
    void painted(QImage);
    void resized(QSize);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};

//-------------------------------------------------------------------------------------------------
class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;

    PaintEventFilter* paintEventFilter;
    GstDisplay* gstDisplay;

private slots:
    void OnPainted(QImage image);
};
#endif // MAINWINDOW_H
