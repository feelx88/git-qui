#include "branchlist.hpp"
#include "ui_branchlist.h"

#include <QInputDialog>
#include <QAction>
#include <QClipboard>
#include <QMenu>

#include "mainwindow.hpp"
#include "core.hpp"
#include "project.hpp"
#include "gitinterface.hpp"
#include "qtreewidgetutils.hpp"
#include "toolbaractions.hpp"

struct BranchListPrivate
{
  BranchList *_this;
  GitInterface *gitInterface = nullptr;
  QFont italicFont;

  BranchListPrivate(BranchList *branchList)
  : _this(branchList)
  {}

  void connectSignals()
  {
    italicFont = _this->ui->treeWidget->font();
    italicFont.setItalic(true);

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, _this, [=](QTreeWidgetItem *item){
      QString branch = item->data(0, Qt::UserRole).toString();
      if (!branch.isEmpty())
      {
        gitInterface->changeBranch(branch);
      }
    });

    _this->connect(_this->ui->treeWidget_2, &QTreeWidget::itemDoubleClicked, _this, [=](QTreeWidgetItem *item){
      QString branch = item->data(0, Qt::UserRole).toString();
      if (!branch.isEmpty())
      {
        QString localBranch = branch.right(branch.length() - branch.indexOf('/') - 1);
        gitInterface->changeBranch(localBranch, branch);
      }
    });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *copyAction = new QAction(_this->tr("Copy branch name"), _this);
    QAction *deleteAction = new QAction(_this->tr("Delete branch"), _this);
    _this->connect(copyAction, &QAction::triggered, _this, [=]{
      QAction *action = static_cast<QAction*>(_this->sender());
      QTreeWidget *treeWidget = static_cast<QTreeWidget*>(action->parentWidget());
      if (!treeWidget->selectedItems().empty())
      {
        QApplication::clipboard()->setText(
              treeWidget->selectedItems().first()->data(0, Qt::UserRole).toString()
        );
      }
    });
    _this->connect(deleteAction, &QAction::triggered, _this, [=]{
      QString branch = _this->ui->treeWidget->selectedItems().first()->data(0, Qt::UserRole).toString();
      if (!branch.isEmpty())
      {
        gitInterface->deleteBranch(branch);
      }
    });
    _this->ui->treeWidget->addAction(ToolBarActions::byId("new-branch"));
    _this->ui->treeWidget->addAction(copyAction);
    _this->ui->treeWidget->addAction(deleteAction);
    _this->ui->treeWidget_2->addAction(copyAction);
  }
};

DOCK_WIDGET_IMPL(
  BranchList,
  tr("Branch list")
)

BranchList::BranchList(MainWindow *mainWindow) :
DockWidget(mainWindow),
ui(new Ui::BranchList),
_impl(new BranchListPrivate(this))
{
  ui->setupUi(this);

  _impl->connectSignals();
}

BranchList::~BranchList()
{
  delete ui;
}

void BranchList::onRepositorySwitched(GitInterface *newGitInterface)
{
  DockWidget::onRepositorySwitched(newGitInterface);

  _impl->gitInterface->disconnect(_impl->gitInterface, &GitInterface::branchesChanged, this, nullptr);

  _impl->gitInterface = newGitInterface;

  _impl->gitInterface->connect(_impl->gitInterface, &GitInterface::branchesChanged, this, [=](const QList<GitBranch> &branches){
    ui->treeWidget->clear();
    ui->treeWidget_2->clear();

    QList<QString> localBranches, remoteBranches;
    QString currentBranch;

    for (auto branch : branches)
    {
      if (branch.active)
      {
        currentBranch = branch.name;
      }

      if (branch.remote)
      {
        remoteBranches.append(branch.name);
      }
      else
      {
        localBranches.append(branch.name);
      }
    }
    ui->treeWidget->addTopLevelItems(TreeWidgetUtils::createItems(ui->treeWidget, localBranches));
    ui->treeWidget_2->addTopLevelItems(TreeWidgetUtils::createItems(ui->treeWidget_2, remoteBranches));
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
      if ((*it)->data(0, Qt::UserRole) == currentBranch)
      {
        auto activeFont = font();
        activeFont.setBold(true);
        (*it)->setFont(0, activeFont);
      }
      (*it)->setExpanded(true);
      ++it;
    }

    ui->treeWidget_2->expandAll();
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget_2->resizeColumnToContents(0);
  });
}
