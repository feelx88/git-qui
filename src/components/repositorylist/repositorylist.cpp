#include "repositorylist.hpp"
#include "ui_repositorylist.h"

#include "core.hpp"
#include "mainwindow.hpp"
#include "project.hpp"
#include "treewidgetitem.hpp"

struct RepositoryListPrivate {
  GitInterface *gitInterface = nullptr;
  QString currentRepository;

  void onActionStarted(QTreeWidgetItem *item,
                       const GitInterface::ActionTag &actionTag) {
    QString text;
    switch (actionTag) {
    case GitInterface::ActionTag::GIT_PUSH:
      text = QObject::tr("Pushing...");
      break;
    case GitInterface::ActionTag::GIT_PULL:
      text = QObject::tr("Pulling...");
      break;
    case GitInterface::ActionTag::GIT_FETCH:
      text = QObject::tr("Fetching...");
      break;
    case GitInterface::ActionTag::GIT_COMMIT:
      text = QObject::tr("Committing...");
      break;
    case GitInterface::ActionTag::GIT_STASH:
      text = QObject::tr("Stashing...");
      break;
    case GitInterface::ActionTag::GIT_STASH_APPLY:
      text = QObject::tr("Restoring stash entry...");
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
};

DOCK_WIDGET_IMPL(RepositoryList, tr("Repository list"))

RepositoryList::RepositoryList(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::RepositoryList),
      _impl(new RepositoryListPrivate) {
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

  connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, newProject, [=] {
    QList<QTreeWidgetItem *> selection = ui->treeWidget->selectedItems();
    if (!selection.isEmpty()) {
      newProject->setCurrentRepository(selection.first()->text(0));
    }
  });
}

void RepositoryList::onRepositoryAdded(GitInterface *newGitInterface) {
  auto items = ui->treeWidget->findItems(newGitInterface->name(),
                                         Qt::MatchCaseSensitive, 0);

  if (!items.isEmpty()) {
    return;
  }

  TreeWidgetItem *item = new TreeWidgetItem(this);
  item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
  item->setText(0, newGitInterface->name());
  item->setData(0, Qt::UserRole, newGitInterface->path());
  item->setTextAlignment(1, Qt::AlignRight);
  ui->treeWidget->addTopLevelItem(item);

  connect(newGitInterface, &GitInterface::branchChanged, item,
          [=](const QString &branch, bool hasChanges, bool hasUpstream,
              int commitsAhead, int commitsBehind) {
            if (hasUpstream) {
              item->setText(1, QString("%1%2 %3↑ %4↓")
                                   .arg(branch)
                                   .arg(hasChanges ? "*" : "")
                                   .arg(commitsAhead)
                                   .arg(commitsBehind));

              QFont font = item->treeWidget()->font();
              font.setBold(commitsAhead > 0 || commitsBehind > 0);
              item->setFont(1, font);
            } else {
              item->setText(
                  1, QString("%1%2 ∅").arg(branch).arg(hasChanges ? "*" : ""));
            }
            ui->treeWidget->resizeColumnToContents(1);
            item->setForeground(1, item->foreground(0));
            item->setIcon(
                0, QIcon::fromTheme("state-ok",
                                    QIcon(":/deploy/icons/state-ok.svg")));
          });

  connect(newGitInterface, &GitInterface::actionStarted, item,
          std::bind(std::mem_fn(&RepositoryListPrivate::onActionStarted),
                    _impl.get(), item, std::placeholders::_1));

  connect(newGitInterface, &GitInterface::error, item,
          [=](const QString &, GitInterface::ActionTag,
              GitInterface::ErrorType type) {
            if (type != GitInterface::ErrorType::ALREADY_RUNNING) {
              item->setIcon(
                  0, QIcon::fromTheme("state-error",
                                      QIcon(":/deploy/icons/state-error.svg")));
            }
          });
}

void RepositoryList::onRepositorySwitched(GitInterface *newGitInterface,
                                          QObject *activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);
  _impl->gitInterface = newGitInterface;
  _impl->currentRepository = newGitInterface->name();
  auto items = ui->treeWidget->findItems(newGitInterface->name(),
                                         Qt::MatchCaseSensitive, 0);
  if (!items.isEmpty()) {
    ui->treeWidget->setCurrentItem(items.first());
  }
}

void RepositoryList::onRepositoryRemoved(GitInterface *newGitInterface) {
  auto items = ui->treeWidget->findItems(newGitInterface->name(),
                                         Qt::MatchCaseSensitive, 0);
  if (!items.isEmpty()) {
    delete items.first();
  }

  disconnect(newGitInterface, &GitInterface::branchChanged, this, nullptr);
}
