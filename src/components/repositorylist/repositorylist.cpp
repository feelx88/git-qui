#include "repositorylist.h"
#include "ui_repositorylist.h"

#include "mainwindow.hpp"

struct RepositoryListPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  void connectSignals(RepositoryList *_this)
  {
    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositoryAdded, _this, [=](const QString &path){
      auto items = _this->ui->listWidget->findItems(path, Qt::MatchCaseSensitive);
      if (items.isEmpty())
      {
        _this->ui->listWidget->addItem(path);
      }
    });

    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositoryRemoved, _this, [=](const QString &path){
      auto items = _this->ui->listWidget->findItems(path, Qt::MatchCaseSensitive);
      if (!items.isEmpty())
      {
        _this->ui->listWidget->removeItemWidget(items.first());
        delete items.first();
      }
    });

    _this->connect(gitInterface.get(), &GitInterface::repositorySwitched, _this, [=](const QString &path){
      auto items = _this->ui->listWidget->findItems(path, Qt::MatchCaseSensitive);
      if (!items.isEmpty())
      {
        _this->ui->listWidget->setCurrentItem(items.first());
      }
    });

    _this->connect(_this->ui->listWidget, &QListWidget::itemSelectionChanged, _this, [=]{
      if (_this->ui->listWidget->currentItem())
      {
        gitInterface->switchRepository(_this->ui->listWidget->currentItem()->text());
      }
    });

    _this->connect(_this->ui->pushButton, &QPushButton::clicked, static_cast<MainWindow*>(_this->parent()), &MainWindow::openRepository);
    _this->connect(_this->ui->pushButton_2, &QPushButton::clicked, static_cast<MainWindow*>(_this->parent()), &MainWindow::closeCurrentRepository);
  }

  static void initialize(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new RepositoryList(mainWindow, gitInterface));
  }

  static void restore(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &)
  {
    RepositoryList *repositoryList = new RepositoryList(mainWindow, gitInterface);
    repositoryList->setObjectName(id);
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, repositoryList);
  }
};

DOCK_WIDGET_IMPL(
    RepositoryList,
    tr("Repository list"),
    &RepositoryListPrivate::initialize,
    &RepositoryListPrivate::restore
)

RepositoryList::RepositoryList(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
  DockWidget(parent),
  ui(new Ui::RepositoryList),
  _impl(new RepositoryListPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;
  _impl->connectSignals(this);
}

RepositoryList::~RepositoryList()
{
  delete ui;
}
