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

      _this->connect(newGitInterface, &GitInterface::pushStarted, _this, [=]{
        auto items = _this->ui->treeWidget->findItems(newGitInterface->path().split('/').last(), Qt::MatchExactly);

        if (!items.empty())
        {
          auto item = items.first();
          item->setDisabled(true);
          item->setText(1, _this->tr("Pushing..."));
        }
      });

      _this->connect(newGitInterface, &GitInterface::pullStarted, _this, [=]{
        auto items = _this->ui->treeWidget->findItems(newGitInterface->path().split('/').last(), Qt::MatchExactly);

        if (!items.empty())
        {
          auto item = items.first();
          item->setDisabled(true);
          item->setText(1, _this->tr("Pulling..."));
        }
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

    _this->connect(_this->mainWindow(), &MainWindow::repositorySwitched, _this, [=](GitInterface *newGitInterface){
      gitInterface = newGitInterface;
    });
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
