#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QToolBar>
#include <QDir>

#include "components/repositoryfiles/repositoryfiles.hpp"

struct MainWindowPrivate
{
  QSharedPointer<GitInterface> gitInterface;

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

    _this->connect(_this->ui->actionRepositoryFiles, &QAction::triggered, [_this, this](){
      _this->addDockWidget(Qt::TopDockWidgetArea, new RepositoryFiles(_this, gitInterface));
      gitInterface->reload();
    });

    _this->connect(_this->ui->actionReload_current_repository, &QAction::triggered, [this]{
      gitInterface->reload();
    });
  }
};

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
_impl(new MainWindowPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface.reset(new GitInterface(this, QDir::current()));
  _impl->connectSignals(this);

  _impl->gitInterface->reload();
}

MainWindow::~MainWindow()
{
  delete ui;
}
