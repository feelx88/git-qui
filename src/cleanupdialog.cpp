#include "cleanupdialog.hpp"
#include "ui_cleanupdialog.h"

#include "core.hpp"
#include "project.hpp"
#include "gitinterface.hpp"
#include "treewidgetitem.hpp"

struct CleanUpDialogPrivate
{
  QList<QTreeWidgetItem*> items;
};

CleanUpDialog::CleanUpDialog(Core *core, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CleanUpDialog),
  _impl(new CleanUpDialogPrivate)
{
  ui->setupUi(this);

  for (auto &repository : core->project()->repositoryList())
  {
    QTreeWidgetItem *repoItem = new TreeWidgetItem(static_cast<QTreeWidget*>(nullptr), {repository->name()}, this);
    for (auto branch : repository->branches({"--merged", "master"}))
    {
      if (branch.name != "master")
      {
        QTreeWidgetItem *item = new TreeWidgetItem(repoItem, {branch.name}, this);
        item->setCheckState(0, Qt::Checked);
        _impl->items.append(item);
      }
    }

    if (repoItem->childCount() > 0)
    {
      repoItem->setCheckState(0, Qt::Checked);
      repoItem->setData(0, Qt::UserRole, repository->name());
      repoItem->setText(1, tr("%1 branch(es)").arg(repoItem->childCount()));
      ui->treeWidget->addTopLevelItem(repoItem);
      _impl->items.append(repoItem);
    }
  }

  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->setHeaderLabels({
    tr(""),
    tr("Count")
  });

  connect(ui->treeWidget, &QTreeWidget::itemChanged, this, [=](QTreeWidgetItem *item, int){
    if (!item->data(0, Qt::UserRole).isNull())
    {
      for (int x = 0; x < item->childCount(); ++x)
      {
        item->child(x)->setCheckState(0, item->checkState(0));
      }
    }
  });

  connect(ui->pushButton_2, &QPushButton::clicked, this, [=]{
    for (auto &item : _impl->items)
    {
      item->setCheckState(0, Qt::Checked);
    }
  });

  connect(ui->pushButton, &QPushButton::clicked, this, [=]{
    for (auto &item : _impl->items)
    {
      item->setCheckState(0, Qt::Unchecked);
    }
  });

  connect(ui->pushButton_3, &QPushButton::clicked, this, [=]{
    for (auto &item : _impl->items)
    {
      if (item->checkState(0) == Qt::Checked && item->data(0, Qt::UserRole).isNull())
      {
        GitInterface *repository = core->project()->repositoryByName(item->parent()->data(0, Qt::UserRole).toString());
        if (repository)
        {
          repository->deleteBranch(item->text(0));
        }
      }
    }
    close();
  });
}

CleanUpDialog::~CleanUpDialog()
{
  delete ui;
}
