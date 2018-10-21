#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QToolBar>
#include <QDir>
#include <QDockWidget>
#include <QDebug>
#include <QSettings>

#include "gitinterface.hpp"
#include "components/dockwidget.hpp"

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

    _this->connect(_this->ui->actionReload_current_repository, &QAction::triggered, [this]{
      gitInterface->reload();
    });
  }

  void populateMenu(MainWindow *_this)
  {
    for (DockWidget::RegistryEntry *entry : DockWidget::registeredDockWidgets())
    {
      _this->ui->menuAdd_view->addAction(entry->name, [=]{
        entry->initializer(_this, gitInterface);
        gitInterface->reload();
      });
    }
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
  _impl->populateMenu(this);

  QSettings settings;
  restoreGeometry(settings.value("geometry").toByteArray());

  for (QAction* action : ui->menuAdd_view->actions())
  {
    action->trigger();

    if(action->text() == "Repository files")
    {
      action->trigger();
    }
  }

  _impl->gitInterface->reload();
}

MainWindow::~MainWindow()
{
  QSettings settings;
  settings.setValue("geometry", saveGeometry());

  for (auto dockWidget : findChildren<QDockWidget *>())
  {
    delete dockWidget;
  }
  delete ui;
}
