#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QToolBar>

#include "components/repositoryfiles/repositoryfiles.hpp"

struct MainWindowPrivate
{
  void connectSignals(MainWindow *_this)
  {
    _this->connect(_this->ui->actionTop, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::TopToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionBottom, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::BottomToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionLeft, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::LeftToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionRight, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::RightToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionRepositoryFiles, &QAction::triggered, [_this](){
      _this->addDockWidget(Qt::TopDockWidgetArea, new RepositoryFiles(_this, nullptr));
    });
  }
};

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
_impl(new MainWindowPrivate)
{
  ui->setupUi(this);

  _impl->connectSignals(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}
