#include "repositorylist.h"
#include "ui_repositorylist.h"

#include "mainwindow.hpp"

struct RepositoryListPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  QString currentRepository;

  void connectSignals(RepositoryList *_this)
  {
    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositoryAdded, _this, [=](const QString &path){
      QString directory = path.split('/').last();
      auto items = _this->ui->treeWidget->findItems(directory, Qt::MatchCaseSensitive, 0);
      if (items.isEmpty())
      {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
        item->setText(0, directory);
        item->setData(0, Qt::UserRole, path);
        item->setTextAlignment(1, Qt::AlignRight);
        _this->ui->treeWidget->addTopLevelItem(item);
      }
    });

    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositoryRemoved, _this, [=](const QString &path){
      auto items = _this->ui->treeWidget->findItems(path.split('/').last(), Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        delete items.first();
      }
    });

    _this->connect(gitInterface.get(), &GitInterface::repositorySwitched, _this, [=](const QString &path){
      currentRepository = path.split('/').last();
      auto items = _this->ui->treeWidget->findItems(currentRepository, Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        _this->ui->treeWidget->setCurrentItem(items.first());
      }
    });

    _this->connect(gitInterface.get(), &GitInterface::branchChanged, _this, [=](const QString &branch, bool hasChanges, bool hasUpstream, int commitsAhead, int commitsBehind){
      auto items = _this->ui->treeWidget->findItems(currentRepository, Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        auto item = items.first();
        if (hasUpstream)
        {
          item->setText(1, QString("%1%2 %3↑ %4↓").arg(branch).arg(hasChanges ? "*" : "").arg(commitsAhead).arg(commitsBehind));
        }
        else
        {
          item->setText(1, QString("%1%2 ∅").arg(branch).arg(hasChanges ? "*" : ""));
        }
        _this->ui->treeWidget->resizeColumnToContents(1);
      }
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemSelectionChanged, _this, [=]{
      if (_this->ui->treeWidget->currentItem())
      {
        gitInterface->switchRepository(_this->ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString());
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

  ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

RepositoryList::~RepositoryList()
{
  delete ui;
}
