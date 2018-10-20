#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolBar>

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connect(ui->actionTop, &QAction::triggered, [this]{
    addToolBar(Qt::TopToolBarArea, new QToolBar(this));
  });

  connect(ui->actionBottom, &QAction::triggered, [this]{
    addToolBar(Qt::BottomToolBarArea, new QToolBar(this));
  });

  connect(ui->actionLeft, &QAction::triggered, [this]{
    addToolBar(Qt::LeftToolBarArea, new QToolBar(this));
  });

  connect(ui->actionRight, &QAction::triggered, [this]{
    addToolBar(Qt::RightToolBarArea, new QToolBar(this));
  });

  connect(ui->actionRepositoryFiles, &QAction::triggered, [this](){
    this->addDockWidget(Qt::TopDockWidgetArea, new RepositoryFiles(this));
  });
}

MainWindow::~MainWindow()
{
  delete ui;
}
