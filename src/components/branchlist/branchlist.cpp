#include "branchlist.hpp"
#include "ui_branchlist.h"

#include <QInputDialog>
#include <QAction>

#include "mainwindow.hpp"
#include "qtreewidgetutils.hpp"

struct BranchListPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  QFont italicFont;

  void connectSignals(BranchList *_this)
  {
    MainWindow *mainWindow = static_cast<MainWindow*>(_this->parent());
    italicFont = _this->ui->treeWidget->font();
    italicFont.setItalic(true);

    _this->connect(mainWindow, &MainWindow::repositorySwitched, _this, [=](const QSharedPointer<GitInterface> newGitInterface){
      gitInterface->disconnect(gitInterface.get(), &GitInterface::branchesChanged, _this, nullptr);

      gitInterface = newGitInterface;

      gitInterface->connect(gitInterface.get(), &GitInterface::branchesChanged, _this, [=](const QList<GitBranch> &branches){
        _this->ui->treeWidget->clear();
        _this->ui->treeWidget_2->clear();

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
        _this->ui->treeWidget->addTopLevelItems(QTreeWidgetUtils::createItems(_this->ui->treeWidget, localBranches));
        _this->ui->treeWidget_2->addTopLevelItems(QTreeWidgetUtils::createItems(_this->ui->treeWidget_2, remoteBranches));
        QTreeWidgetItemIterator it(_this->ui->treeWidget);
        while (*it)
        {
          if ((*it)->data(0, Qt::UserRole) == currentBranch)
          {
            auto font = _this->font();
            font.setBold(true);
            (*it)->setFont(0, font);
          }
          (*it)->setExpanded(true);
          ++it;
        }

        _this->ui->treeWidget_2->expandAll();
        _this->ui->treeWidget->resizeColumnToContents(0);
        _this->ui->treeWidget_2->resizeColumnToContents(0);
      });
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, _this, [=](QTreeWidgetItem *item){
      QString branch = item->data(0, Qt::UserRole).toString();
      if (!branch.isEmpty())
      {
        gitInterface->changeBranch(branch);
      }
    });

    _this->connect(_this->ui->createBranchButton, &QPushButton::clicked, _this, [=]{
      gitInterface->createBranch(QInputDialog::getText(_this, _this->tr("Create new branch"), _this->tr("New branch name")));
    });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *deleteAction = new QAction(_this->tr("Delete branch"), _this);
    _this->connect(deleteAction, &QAction::triggered, _this, [=]{
      QString branch = _this->ui->treeWidget->selectedItems().first()->data(0, Qt::UserRole).toString();
      if (!branch.isEmpty())
      {
        gitInterface->deleteBranch(branch);
      }
    });
    _this->ui->treeWidget->addAction(deleteAction);
  }
};

DOCK_WIDGET_IMPL(
  BranchList,
  tr("Branch list")
)

BranchList::BranchList(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
DockWidget(parent),
ui(new Ui::BranchList),
_impl(new BranchListPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;
  _impl->connectSignals(this);
}

BranchList::~BranchList()
{
  delete ui;
}
