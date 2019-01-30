#include "repositorylist.hpp"
#include "ui_repositorylist.h"

#include <QtConcurrent/QtConcurrent>

#include "mainwindow.hpp"

struct RepositoryListPrivate
{
  GitInterface *gitInterface;
  QString currentRepository;

  void connectSignals(RepositoryList *_this)
  {
    _this->connect(_this->mainWindow(), &MainWindow::repositoryAdded, _this, [=](GitInterface *newGitInterface){
      QString directory = newGitInterface->path().split('/').last();
      auto items = _this->ui->treeWidget->findItems(directory, Qt::MatchCaseSensitive, 0);

      if (!items.isEmpty())
      {
        return;
      }

      QTreeWidgetItem *item = new QTreeWidgetItem;
      item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
      item->setText(0, directory);
      item->setData(0, Qt::UserRole, newGitInterface->path());
      item->setTextAlignment(1, Qt::AlignRight);
      _this->ui->treeWidget->addTopLevelItem(item);

      _this->connect(newGitInterface, &GitInterface::branchChanged, _this, [=](const QString &branch, bool hasChanges, bool hasUpstream, int commitsAhead, int commitsBehind){
        if (hasUpstream)
        {
          item->setText(1, QString("%1%2 %3↑ %4↓").arg(branch).arg(hasChanges ? "*" : "").arg(commitsAhead).arg(commitsBehind));
        }
        else
        {
          item->setText(1, QString("%1%2 ∅").arg(branch).arg(hasChanges ? "*" : ""));
        }
        _this->ui->treeWidget->resizeColumnToContents(1);
      });
    });

    _this->connect(_this->mainWindow(), &MainWindow::repositoryRemoved, _this, [=](GitInterface *newGitInterface){
      auto items = _this->ui->treeWidget->findItems(newGitInterface->path().split('/').last(), Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        delete items.first();
      }

      _this->disconnect(newGitInterface, &GitInterface::branchChanged, _this, nullptr);
    });

    _this->connect(_this->mainWindow(), &MainWindow::repositorySwitched, _this, [=](GitInterface *newGitInterface){
      currentRepository = newGitInterface->path().split('/').last();
      auto items = _this->ui->treeWidget->findItems(currentRepository, Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        _this->ui->treeWidget->setCurrentItem(items.first());
      }
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemSelectionChanged, _this, [=]{
      if (_this->ui->treeWidget->currentItem())
      {
        _this->mainWindow()->switchRepository(_this->ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString());
      }
    });

    _this->connect(_this->ui->pushButton, &QPushButton::clicked, _this->mainWindow(), &MainWindow::openRepository);
    _this->connect(_this->ui->pushButton_2, &QPushButton::clicked, _this->mainWindow(), &MainWindow::closeCurrentRepository);

    _this->connect(_this->mainWindow(), &MainWindow::repositorySwitched, _this, [=](GitInterface *newGitInterface){
      gitInterface = newGitInterface;
    });

    _this->ui->toolButton->addAction(_this->ui->actionPull_all_repositories);
    _this->connect(_this->ui->toolButton, &QToolButton::clicked, _this, [=]{
      startBackgroundAction(_this, gitInterface, false);
    });
    _this->connect(_this->ui->actionPull_all_repositories, &QAction::triggered, _this, [=]{
      for (auto repo : _this->mainWindow()->repositories())
      {
        startBackgroundAction(_this, repo, false);
      }
    });

    _this->ui->toolButton_2->addAction(_this->ui->actionPush_all_repositories);
    _this->connect(_this->ui->toolButton_2, &QToolButton::clicked, _this, [=]{
      startBackgroundAction(_this, gitInterface, true);
    });
    _this->connect(_this->ui->actionPush_all_repositories, &QAction::triggered, _this, [=]{
      for (auto repo : _this->mainWindow()->repositories())
      {
        startBackgroundAction(_this, repo, true);
      }
    });
  }

  void startBackgroundAction(RepositoryList *_this, GitInterface *gitInterface, bool push)
  {
    auto items = _this->ui->treeWidget->findItems(gitInterface->path().split('/').last(), Qt::MatchExactly);

    if (!items.empty())
    {
      auto item = items.first();
      item->setDisabled(true);
      item->setText(1, push ? _this->tr("Pushing...") : _this->tr("Pulling..."));
      QtConcurrent::run([=]{
        if (push)
        {
          gitInterface->push();
        }
        else
        {
          gitInterface->pull(true);
        }
        item->setDisabled(false);
      });
    }
  }
};

DOCK_WIDGET_IMPL(
    RepositoryList,
    tr("Repository list")
)

RepositoryList::RepositoryList(MainWindow *mainWindow, GitInterface *gitInterface) :
  DockWidget(mainWindow),
  ui(new Ui::RepositoryList),
  _impl(new RepositoryListPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;
  _impl->currentRepository = gitInterface->path();
  _impl->connectSignals(this);

  ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

RepositoryList::~RepositoryList()
{
  delete ui;
}
