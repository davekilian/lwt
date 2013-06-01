#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setMouseTracking(true);

    ui->widget->setFocusPolicy(Qt::StrongFocus);
    ui->widget->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}
