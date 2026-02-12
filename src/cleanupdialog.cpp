#include "cleanupdialog.hpp"
#include "ui_cleanupdialog.h"

#include <QFutureWatcher>
#include <QMap>

#include "core.hpp"
#include "gitinterface.hpp"
#include "project.hpp"
#include "treewidgetitem.hpp"

struct CleanUpDialogPrivate {
  QList<QTreeWidgetItem *> items;
};

CleanUpDialog::CleanUpDialog(Core *core, QWidget *parent)
    : QDialog(parent), ui(new Ui::CleanUpDialog),
      _impl(new CleanUpDialogPrivate) {
  ui->setupUi(this);

  auto repositories = core->project()->repositoryList();

  for (auto &repository : repositories) {
    QTreeWidgetItem *repoItem = new TreeWidgetItem(
        static_cast<QTreeWidget *>(nullptr), {repository->name()}, this);
    repoItem->setDisabled(true);
    auto branchFuture = repository->branch({"--merged", "master"});

    if (branchFuture.isCanceled()) {
      return;
    }

    auto watcher = new QFutureWatcher<QList<GitBranch>>(this);
    connect(watcher, &QFutureWatcher<QList<GitBranch>>::finished, this,
            [=, this] {
              for (auto &branch : watcher->future().result()) {
                if (branch.name != "master") {
                  QTreeWidgetItem *item =
                      new TreeWidgetItem(repoItem, {branch.name}, this);
                  item->setCheckState(0, Qt::Checked);
                  _impl->items.append(item);
                }
              }

              if (repoItem->childCount() > 0) {
                repoItem->setCheckState(0, Qt::Checked);
                repoItem->setData(0, Qt::UserRole, repository->name());
                repoItem->setText(
                    1, tr("%1 branch(es)").arg(repoItem->childCount()));
                ui->treeWidget->addTopLevelItem(repoItem);
                _impl->items.append(repoItem);
                repoItem->setExpanded(true);
              }
              repoItem->setDisabled(false);
              ui->treeWidget->resizeColumnToContents(0);
            });
    connect(watcher, &QFutureWatcher<QList<GitBranch>>::finished, watcher,
            &QFutureWatcher<QList<GitBranch>>::deleteLater);
    watcher->setFuture(branchFuture);
  }

  ui->treeWidget->setHeaderLabels({tr(""), tr("Count")});

  connect(ui->treeWidget, &QTreeWidget::itemChanged, this,
          [=, this](QTreeWidgetItem *item, int) {
            if (!item->data(0, Qt::UserRole).isNull()) {
              for (int x = 0; x < item->childCount(); ++x) {
                item->child(x)->setCheckState(0, item->checkState(0));
              }
            }
          });

  connect(ui->pushButton_2, &QPushButton::clicked, this, [=, this] {
    for (auto &item : _impl->items) {
      item->setCheckState(0, Qt::Checked);
    }
  });

  connect(ui->pushButton, &QPushButton::clicked, this, [=, this] {
    for (auto &item : _impl->items) {
      item->setCheckState(0, Qt::Unchecked);
    }
  });

  connect(ui->pushButton_3, &QPushButton::clicked, this, [=, this] {
    QMap<QSharedPointer<GitInterface>, QStringList> branchesToDelete;
    for (auto &item : _impl->items) {
      if (item->checkState(0) == Qt::Checked &&
          item->data(0, Qt::UserRole).isNull()) {
        auto repository = core->project()->repositoryByName(
            item->parent()->data(0, Qt::UserRole).toString());
        if (repository) {
          branchesToDelete[repository] << item->text(0);
        }
      }
    }

    for (auto [repository, branches] : branchesToDelete.asKeyValueRange()) {
      repository->deleteBranches(branches);
    }
    close();
  });
}

CleanUpDialog::~CleanUpDialog() { delete ui; }
