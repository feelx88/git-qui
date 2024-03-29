#include "branchlist.hpp"
#include "ui_branchlist.h"

#include <QAction>
#include <QClipboard>
#include <QInputDialog>
#include <QMenu>

#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "project.hpp"
#include "qtreewidgetutils.hpp"
#include "toolbaractions.hpp"

using namespace std::placeholders;

struct BranchListPrivate {
  BranchList *_this;
  QSharedPointer<GitInterface> gitInterface;
  QFont italicFont;

  BranchListPrivate(BranchList *branchList) : _this(branchList) {}

  void connectSignals() {
    italicFont = _this->ui->treeWidget->font();
    italicFont.setItalic(true);

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked,
                   _this, [=, this](QTreeWidgetItem *item) {
                     QString branch = item->data(0, Qt::UserRole).toString();
                     if (!branch.isEmpty()) {
                       gitInterface->changeBranch(branch);
                     }
                   });

    _this->connect(_this->ui->treeWidget_2, &QTreeWidget::itemDoubleClicked,
                   _this, [=, this](QTreeWidgetItem *item) {
                     QString branch = item->data(0, Qt::UserRole).toString();
                     if (!branch.isEmpty()) {
                       QString localBranch = branch.right(
                           branch.length() - branch.indexOf('/') - 1);
                       gitInterface->changeBranch(localBranch, branch);
                     }
                   });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *copyAction =
        new QAction(BranchList::tr("Copy branch name"), _this);
    QAction *deleteAction = new QAction(BranchList::tr("Delete branch"), _this);
    _this->connect(copyAction, &QAction::triggered, _this, [=, this] {
      if (!_this->ui->treeWidget->selectedItems().empty()) {
        QApplication::clipboard()->setText(
            _this->ui->treeWidget->selectedItems()
                .first()
                ->data(0, Qt::UserRole)
                .toString());
      }
    });
    _this->connect(deleteAction, &QAction::triggered, _this, [=, this] {
      QString branch = _this->ui->treeWidget->selectedItems()
                           .first()
                           ->data(0, Qt::UserRole)
                           .toString();
      if (!branch.isEmpty()) {
        gitInterface->deleteBranch(branch);
      }
    });
    _this->ui->treeWidget->addAction(
        ToolBarActions::byId(ToolBarActions::ActionID::NEW_BRANCH));
    _this->ui->treeWidget->addAction(copyAction);
    _this->ui->treeWidget->addAction(deleteAction);
    _this->ui->treeWidget_2->addAction(copyAction);
  }

  void refreshView(const QList<GitBranch> &branches) {
    _this->ui->treeWidget->clear();
    _this->ui->treeWidget_2->clear();

    QList<QString> localBranches, remoteBranches;
    QString currentBranch;

    for (const auto &branch : branches) {
      if (branch.active) {
        currentBranch = branch.name;
      }

      if (branch.remote) {
        remoteBranches.append(branch.name);
      } else {
        localBranches.append(branch.name);
      }
    }
    TreeWidgetUtils::createItems(_this->ui->treeWidget, localBranches);
    TreeWidgetUtils::createItems(_this->ui->treeWidget_2, remoteBranches);
    QTreeWidgetItemIterator it(_this->ui->treeWidget);
    while (*it) {
      if ((*it)->data(0, Qt::UserRole) == currentBranch) {
        auto activeFont = _this->font();
        activeFont.setBold(true);
        (*it)->setFont(0, activeFont);
      }
      (*it)->setExpanded(true);
      ++it;
    }

    _this->ui->treeWidget_2->expandAll();
    _this->ui->treeWidget->resizeColumnToContents(0);
    _this->ui->treeWidget_2->resizeColumnToContents(0);
  }
};

DOCK_WIDGET_IMPL(BranchList, BranchList::tr("Branch list"))

BranchList::BranchList(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::BranchList),
      _impl(new BranchListPrivate(this)) {
  ui->setupUi(this);

  _impl->connectSignals();
}

BranchList::~BranchList() { delete ui; }

void BranchList::onProjectSwitched(Project *newProject) {
  _impl->gitInterface = nullptr;
  DockWidget::onProjectSwitched(newProject);
}

void BranchList::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);

  _impl->gitInterface = newGitInterface;

  connect(newGitInterface.get(), &GitInterface::branchesChanged,
          activeRepositoryContext.get(),
          [this](QList<GitBranch> branches) { _impl->refreshView(branches); });

  _impl->refreshView(newGitInterface->branches());
}
