#include "branchlist.hpp"
#include "ui_branchlist.h"

#include <QInputDialog>
#include <QAction>

#include "mainwindow.hpp"

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
        for (auto branch : branches)
        {
          auto item = new QTreeWidgetItem(_this->ui->treeWidget, {
            branch.active ? "➔" : "",
            branch.name,
            branch.upstreamName
          });
          item->setFont(2, italicFont);
          _this->ui->treeWidget->addTopLevelItem(item);
        }
        _this->ui->treeWidget->resizeColumnToContents(0);
        _this->ui->treeWidget->resizeColumnToContents(1);
        _this->ui->treeWidget->resizeColumnToContents(2);
      });
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, _this, [=](QTreeWidgetItem *item){
      gitInterface->changeBranch(item->text(1));
    });

    _this->connect(_this->ui->createBranchButton, &QPushButton::clicked, _this, [=]{
      gitInterface->createBranch(QInputDialog::getText(_this, _this->tr("Create new branch"), _this->tr("New branch name")));
    });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *deleteAction = new QAction(_this->tr("Delete branch"), _this);
    _this->connect(deleteAction, &QAction::triggered, _this, [=]{
      gitInterface->deleteBranch(_this->ui->treeWidget->selectedItems().first()->text(1));
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
