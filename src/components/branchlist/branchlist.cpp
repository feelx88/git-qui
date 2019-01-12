#include "branchlist.hpp"
#include "ui_branchlist.h"

#include "mainwindow.hpp"

struct BranchListPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  void connectSignals(BranchList *_this)
  {
    MainWindow *mainWindow = static_cast<MainWindow*>(_this->parent());

    _this->connect(mainWindow, &MainWindow::repositorySwitched, _this, [=](const QSharedPointer<GitInterface> newGitInterface){
      gitInterface->disconnect(gitInterface.get(), &GitInterface::branchesChanged, _this, nullptr);

      gitInterface = newGitInterface;

      gitInterface->connect(gitInterface.get(), &GitInterface::branchesChanged, _this, [=](const QList<GitBranch> &branches){
        _this->ui->treeWidget->clear();
        for (auto branch : branches)
        {
          _this->ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(_this->ui->treeWidget, {
            branch.active ? "➔" : "",
            branch.name,
            branch.upstreamName
          }));
        }
        _this->ui->treeWidget->resizeColumnToContents(0);
        _this->ui->treeWidget->resizeColumnToContents(1);
        _this->ui->treeWidget->resizeColumnToContents(2);
      });
    });
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
