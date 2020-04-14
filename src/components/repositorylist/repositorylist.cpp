#include "repositorylist.hpp"
#include "ui_repositorylist.h"

#include <QtConcurrent/QtConcurrent>

#include "mainwindow.hpp"
#include "project.hpp"

struct RepositoryListPrivate
{
  GitInterface *gitInterface = nullptr;
  QString currentRepository;

  void connectSignals(RepositoryList *_this)
  {
    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemSelectionChanged, _this, [=]{
      QList<QTreeWidgetItem*> selection = _this->ui->treeWidget->selectedItems();
      if (!selection.isEmpty())
      {
        _this->project()->setCurrentRepository(selection.first()->text(0));
      }
    });
  }
};

DOCK_WIDGET_IMPL(
    RepositoryList,
    tr("Repository list")
)

RepositoryList::RepositoryList(MainWindow *mainWindow) :
  DockWidget(mainWindow),
  ui(new Ui::RepositoryList),
  _impl(new RepositoryListPrivate)
{
  ui->setupUi(this);

  _impl->connectSignals(this);

  ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

RepositoryList::~RepositoryList()
{
  delete ui;
}

void RepositoryList::onProjectSwitched(Project *project)
{
  ui->treeWidget->clear();

  DockWidget::onProjectSwitched(project);
  for (auto &repository : project->repositoryList())
  {
    onRepositoryAdded(repository);
  }

  onRepositorySwitched(project->activeRepository());
}

void RepositoryList::onRepositoryAdded(GitInterface *newGitInterface)
{
  auto items = ui->treeWidget->findItems(newGitInterface->name(), Qt::MatchCaseSensitive, 0);

  if (!items.isEmpty())
  {
    return;
  }

  QTreeWidgetItem *item = new QTreeWidgetItem;
  item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
  item->setText(0, newGitInterface->name());
  item->setData(0, Qt::UserRole, newGitInterface->path());
  item->setTextAlignment(1, Qt::AlignRight);
  ui->treeWidget->addTopLevelItem(item);

  connect(newGitInterface, &GitInterface::branchChanged, newGitInterface, [=](const QString &branch, bool hasChanges, bool hasUpstream, int commitsAhead, int commitsBehind){
    if (hasUpstream)
    {
      item->setText(1, QString("%1%2 %3↑ %4↓").arg(branch).arg(hasChanges ? "*" : "").arg(commitsAhead).arg(commitsBehind));
    }
    else
    {
      item->setText(1, QString("%1%2 ∅").arg(branch).arg(hasChanges ? "*" : ""));
    }
    ui->treeWidget->resizeColumnToContents(1);
    item->setDisabled(false);
    item->setIcon(0, QIcon::fromTheme("state-ok", QIcon(":/deploy/icons/state-ok.svg")));
  });

  connect(newGitInterface, &GitInterface::pushStarted, newGitInterface, [=]{
    auto items = ui->treeWidget->findItems(newGitInterface->name(), Qt::MatchExactly);

    if (!items.empty())
    {
      auto item = items.first();
      item->setDisabled(true);
      item->setText(1, tr("Pushing..."));
      item->setIcon(0, QIcon::fromTheme("state-sync", QIcon(":/deploy/icons/state-sync.svg")));
    }
  });

  connect(newGitInterface, &GitInterface::pullStarted, newGitInterface, [=]{
    auto items = ui->treeWidget->findItems(newGitInterface->name(), Qt::MatchExactly);

    if (!items.empty())
    {
      auto item = items.first();
      item->setDisabled(true);
      item->setText(1, tr("Pulling..."));
      item->setIcon(0, QIcon::fromTheme("state-sync", QIcon(":/deploy/icons/state-sync.svg")));
    }
  });

  connect(newGitInterface, &GitInterface::error, newGitInterface, [=]{
    item->setIcon(0, QIcon::fromTheme("state-error", QIcon(":/deploy/icons/state-error.svg")));
  });
}

void RepositoryList::onRepositorySwitched(GitInterface *newGitInterface)
{
  _impl->gitInterface = newGitInterface;
  _impl->currentRepository = newGitInterface->name();
  auto items = ui->treeWidget->findItems(newGitInterface->name(), Qt::MatchCaseSensitive, 0);
  if (!items.isEmpty())
  {
    ui->treeWidget->setCurrentItem(items.first());
  }
}

void RepositoryList::onRepositoryRemoved(GitInterface *newGitInterface)
{
  auto items = ui->treeWidget->findItems(newGitInterface->name(), Qt::MatchCaseSensitive, 0);
  if (!items.isEmpty())
  {
    delete items.first();
  }

  disconnect(newGitInterface, &GitInterface::branchChanged, this, nullptr);
}
