#include "repositorylist.hpp"
#include "ui_repositorylist.h"

#include "mainwindow.hpp"
#include "project.hpp"
#include "treewidgetitem.hpp"

using namespace std::placeholders;

struct RepositoryListPrivate {
  QSharedPointer<GitInterface> gitInterface = nullptr;
  QString currentRepository;
  RepositoryList *_this;

  RepositoryListPrivate(RepositoryList *_this) : _this(_this) {}

  void onActionStarted(QTreeWidgetItem *item,
                       const GitInterface::ActionTag &actionTag) {
    QString text;
    switch (actionTag) {
    case GitInterface::ActionTag::GIT_PUSH:
      text = RepositoryList::tr("Pushing...");
      break;
    case GitInterface::ActionTag::GIT_PULL:
      text = RepositoryList::tr("Pulling...");
      break;
    case GitInterface::ActionTag::GIT_FETCH:
      text = RepositoryList::tr("Fetching...");
      break;
    case GitInterface::ActionTag::GIT_COMMIT:
      text = RepositoryList::tr("Committing...");
      break;
    case GitInterface::ActionTag::GIT_STASH:
      text = RepositoryList::tr("Stashing...");
      break;
    case GitInterface::ActionTag::GIT_STASH_APPLY:
      text = RepositoryList::tr("Restoring stash entry...");
      break;
    default:
      return;
    }

    auto font = item->font(0);
    font.setItalic(true);

    item->setText(1, text);
    item->setIcon(0, QIcon::fromTheme("state-sync",
                                      QIcon(":/deploy/icons/state-sync.svg")));
    item->setFont(1, font);
    item->setForeground(1, QBrush(item->treeWidget()->palette().color(
                               QPalette::Disabled, QPalette::Text)));
  }

  void onActionFinished(TreeWidgetItem *item,
                        QSharedPointer<GitInterface> gitInterface) {
    onBranchChanged(item, gitInterface->activeBranch());
  }

  void onBranchChanged(TreeWidgetItem *item, const GitBranch &branch) {
    if (branch.hasUpstream) {
      item->setText(1, QString("%1%2 %3↑ %4↓")
                           .arg(branch.name, branch.hasChanges ? "*" : "")
                           .arg(branch.commitsAhead)
                           .arg(branch.commitsBehind));

      QFont font = item->treeWidget()->font();
      font.setBold(branch.commitsAhead > 0 || branch.commitsBehind > 0);
      item->setFont(1, font);
    } else {
      item->setText(
          1, QString("%1%2 ∅").arg(branch.name, branch.hasChanges ? "*" : ""));
    }
    item->setTextAlignment(0, Qt::AlignVCenter | Qt::AlignLeading);
    item->setTextAlignment(1, Qt::AlignVCenter | Qt::AlignTrailing);
    _this->ui->treeWidget->resizeColumnToContents(1);
    item->setForeground(1, item->foreground(0));
    item->setIcon(
        0, QIcon::fromTheme("state-ok", QIcon(":/deploy/icons/state-ok.svg")));
  }
};

DOCK_WIDGET_IMPL(RepositoryList, RepositoryList::tr("Repository list"))

RepositoryList::RepositoryList(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::RepositoryList),
      _impl(new RepositoryListPrivate(this)) {
  ui->setupUi(this);
  ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

RepositoryList::~RepositoryList() { delete ui; }

void RepositoryList::onProjectSwitched(Project *newProject) {
  ui->treeWidget->clear();

  DockWidget::onProjectSwitched(newProject);
  for (auto &repository : newProject->repositoryList()) {
    onRepositoryAdded(repository);
  }

  onRepositorySwitched(newProject->activeRepository(),
                       newProject->activeRepositoryContext());

  connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, newProject,
          [=, this] {
            QList<QTreeWidgetItem *> selection =
                ui->treeWidget->selectedItems();
            if (!selection.isEmpty()) {
              newProject->setCurrentRepository(selection.first()->text(0));
            }
          });
}

void RepositoryList::onRepositoryAdded(
    QSharedPointer<GitInterface> newGitInterface) {
  auto items = ui->treeWidget->findItems(newGitInterface->name(),
                                         Qt::MatchCaseSensitive, 0);

  if (!items.isEmpty()) {
    return;
  }

  TreeWidgetItem *item = new TreeWidgetItem(this);
  item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
  item->setText(0, newGitInterface->name());
  item->setData(0, Qt::UserRole, newGitInterface->path());
  item->setTextAlignment(0, Qt::AlignVCenter | Qt::AlignLeading);
  item->setTextAlignment(1, Qt::AlignVCenter | Qt::AlignTrailing);
  ui->treeWidget->addTopLevelItem(item);

  connect(newGitInterface.get(), &GitInterface::branchChanged, item,
          [=, this](const GitBranch &branch) {
            _impl->onBranchChanged(item, branch);
          });

  connect(newGitInterface.get(), &GitInterface::actionStarted, item,
          [=, this](const GitInterface::ActionTag &actionTag) {
            _impl->onActionStarted(item, actionTag);
          });

  connect(newGitInterface.get(), &GitInterface::actionFinished, item,
          [=, this] { _impl->onActionFinished(item, newGitInterface); });

  _impl->onActionFinished(item, newGitInterface);

  connect(newGitInterface.get(), &GitInterface::error, item,
          [=, this](const QString &, GitInterface::ActionTag,
                    GitInterface::ErrorType type) {
            if (type != GitInterface::ErrorType::ALREADY_RUNNING) {
              item->setIcon(
                  0, QIcon::fromTheme("state-error",
                                      QIcon(":/deploy/icons/state-error.svg")));
            }
          });
}

void RepositoryList::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);
  _impl->gitInterface = newGitInterface;
  _impl->currentRepository = newGitInterface->name();
  auto items = ui->treeWidget->findItems(newGitInterface->name(),
                                         Qt::MatchCaseSensitive, 0);
  if (!items.isEmpty()) {
    ui->treeWidget->setCurrentItem(items.first());
  }
}

void RepositoryList::onRepositoryRemoved(
    QSharedPointer<GitInterface> newGitInterface) {
  auto items = ui->treeWidget->findItems(newGitInterface->name(),
                                         Qt::MatchCaseSensitive, 0);
  if (!items.isEmpty()) {
    delete items.first();
  }

  disconnect(newGitInterface.get(), &GitInterface::branchChanged, this,
             nullptr);
}
