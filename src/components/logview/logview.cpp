#include "logview.hpp"
#include "ui_logview.h"

#include "gitcommit.hpp"
#include "gitinterface.hpp"
#include "graphdelegate.h"
#include "mainwindow.hpp"
#include "toolbaractions.hpp"
#include "treewidgetitem.hpp"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>

QString baseButtonStyleSheet =
    "color: black; padding: 0.1em; background-color: %1; font-weight: %2";
QString tagButtonStyleSheet = baseButtonStyleSheet.arg("yellow", "normal");
QString localBranchButtonStyleSheet =
    baseButtonStyleSheet.arg("lightgray", "normal");
QString localHeadBranchButtonStyleSheet =
    baseButtonStyleSheet.arg("lightgray", "bold");
QString remoteBranchButtonStyleSheet =
    baseButtonStyleSheet.arg("gray", "normal");

struct LogViewPrivate {
  QSharedPointer<GitInterface> gitInterface = nullptr;
  GraphDelegate *graphDelegate;
  QAction *resetAction, *checkoutAction, *deleteAction;
  QMenu *branchMenu;

  LogViewPrivate(LogView *_this) : graphDelegate(new GraphDelegate(_this)) {}

  void connectSignals(LogView *_this) {
    _this->ui->treeWidget->setHeaderLabels({
        _this->tr("Graph"),
        _this->tr("Refs"),
        _this->tr("Message"),
        _this->tr("Author"),
        _this->tr("Date"),
        _this->tr("Id"),
    });

    QObject::connect(
        _this->ui->treeWidget, &QTreeWidget::currentItemChanged, _this,
        [=](QTreeWidgetItem *item) {
          if (item) {
            auto commit = _this->ui->treeWidget->currentItem()
                              ->data(0, 0)
                              .value<GitCommit>();
            _this->ui->treeWidget->setProperty(
                ToolBarActions::ActionCallerProperty::NEW_BRANCH_BASE_COMMIT,
                QVariant::fromValue(commit.id));
            _this->ui->treeWidget->setProperty(
                ToolBarActions::ActionCallerProperty::RESET_REF,
                QVariant::fromValue(commit.id));
          }
        });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    resetAction = new QAction("");
    _this->ui->treeWidget->addAction(resetAction);
    ToolBarActions::connectById(ToolBarActions::ActionID::RESET, resetAction);

    _this->ui->treeWidget->addAction(
        ToolBarActions::byId(ToolBarActions::ActionID::NEW_BRANCH));

    branchMenu = new QMenu(_this);
    checkoutAction = branchMenu->addAction("");
    QObject::connect(checkoutAction, &QAction::triggered, branchMenu, [=] {
      gitInterface->changeBranch(
          branchMenu->property("branch").value<GitRef>().name);
    });

    deleteAction = branchMenu->addAction("");
    QObject::connect(deleteAction, &QAction::triggered, branchMenu, [=] {
      auto branch = branchMenu->property("branch").value<GitRef>().name;
      if (QMessageBox::question(_this, "Delete branch",
                                QString("Delete branch %1?").arg(branch)) ==
          QMessageBox::Yes) {
        gitInterface->deleteBranch(branch);
      }
    });
  }
};

DOCK_WIDGET_IMPL(LogView, tr("Log view"))

LogView::LogView(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::LogView),
      _impl(new LogViewPrivate(this)) {
  ui->setupUi(this);
  ui->treeWidget->setItemDelegateForColumn(0, _impl->graphDelegate);
  ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);

  _impl->connectSignals(this);
}

LogView::~LogView() { delete ui; }

QVariant LogView::configuration() {
  QList<QVariant> widths;

  for (int x = 0; x < ui->treeWidget->columnCount(); ++x) {
    widths << ui->treeWidget->columnWidth(x);
  }

  return widths;
}

void LogView::configure(const QVariant &configuration) {
  QList<QVariant> widths = configuration.toList();
  for (int x = 0; x < widths.size(); ++x) {
    ui->treeWidget->setColumnWidth(x, widths.at(x).toInt());
  }
}

void LogView::onProjectSwitched(Project *newProject) {
  _impl->gitInterface = nullptr;
  DockWidget::onProjectSwitched(newProject);
}

void LogView::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);
  _impl->gitInterface = newGitInterface;
  connect(
      newGitInterface.get(), &GitInterface::logChanged,
      activeRepositoryContext.get(), [=](QSharedPointer<GitTree> tree) {
        ui->treeWidget->clear();
        QList<GraphDelegate::RowInfo> rows;

        for (auto it = tree->commitList().rbegin();
             it != tree->commitList().rend(); ++it) {

          auto commit = *it;
          GraphDelegate::RowInfo currentRow;
          if (rows.isEmpty()) {
            currentRow.childColumns.insert(commit->id, 0);
          } else {
            currentRow = GraphDelegate::RowInfo(rows.first());
          }

          currentRow.currentColumns = currentRow.childColumns;

          auto childIdValues = currentRow.childColumns.values(commit->id);
          auto minChildColumn =
              std::min_element(childIdValues.begin(), childIdValues.end());
          currentRow.column =
              minChildColumn != childIdValues.end() ? *minChildColumn : 0;

          currentRow.currentColumns.remove(currentRow.commitId);
          currentRow.commitId = commit->id;
          currentRow.childColumns.remove(commit->id);

          for (int column = 0; column < commit->childCommits.size(); ++column) {
            auto childCommit = commit->childCommits.at(column).lock();
            if (!currentRow.childColumns.contains(childCommit->id)) {
              int newColumn = 0;
              auto childColumns = currentRow.childColumns.values();
              while (childColumns.contains(newColumn)) {
                ++newColumn;
              }
              currentRow.childColumns.insert(childCommit->id, newColumn);
            }
          }

          auto currentIdValues = currentRow.currentColumns.values();
          currentRow.columnCount = *std::max_element(currentIdValues.begin(),
                                                     currentIdValues.end()) +
                                   1;

          rows.prepend(currentRow);
        }

        for (const auto &commit : tree->commitList()) {
          TreeWidgetItem *item = new TreeWidgetItem(ui->treeWidget);
          item->setText(2, commit->message);
          item->setToolTip(2, commit->message);
          item->setText(3, commit->author);
          item->setText(4, commit->date.toString());
          item->setText(5, commit->id);
          item->setToolTip(5, commit->id);

          item->setData(0, 0, QVariant::fromValue(GitCommit(*commit)));

          ui->treeWidget->addTopLevelItem(item);
        }

        for (int x = 0; x < ui->treeWidget->topLevelItemCount(); ++x) {
          auto item = ui->treeWidget->topLevelItem(x);
          auto commit = item->data(0, 0).value<GitCommit>();

          if (commit.refs.isEmpty()) {
            continue;
          }

          auto container = new QWidget(ui->treeWidget);
          auto layout = new QHBoxLayout(container);
          int remoteIndex = 0;
          layout->setAlignment(Qt::AlignLeft);
          layout->setMargin(2);
          for (const auto &ref : qAsConst(commit.refs)) {
            remoteIndex += ref.isRemote ? 1 : 0;
            int insertPosition = -1;
            auto button = new QPushButton(ref.name, container);
            if (ref.isTag) {
              button->setStyleSheet(tagButtonStyleSheet);
            } else if (ref.isRemote) {
              button->setStyleSheet(remoteBranchButtonStyleSheet);
            } else if (ref.isHead) {
              insertPosition = 0;
              button->setStyleSheet(localHeadBranchButtonStyleSheet);
            } else {
              insertPosition = remoteIndex;
              button->setStyleSheet(localBranchButtonStyleSheet);
            }
            button->setSizePolicy(
                QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
            button->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(button, &QPushButton::customContextMenuRequested, button,
                    [=](const QPoint &at) {
                      ui->treeWidget->clearSelection();
                      item->setSelected(true);

                      if (!ref.isTag && !ref.isRemote) {
                        _impl->branchMenu->setProperty(
                            "commit", QVariant::fromValue(commit));
                        _impl->branchMenu->setProperty(
                            "branch", QVariant::fromValue(ref));
                        _impl->checkoutAction->setText(
                            tr("Check out branch %1").arg(ref.name));
                        _impl->deleteAction->setText(
                            tr("Delete branch %1...").arg(ref.name));
                        _impl->deleteAction->setDisabled(ref.isHead);
                        _impl->branchMenu->popup(button->mapToGlobal(at));
                      }
                    });
            layout->insertWidget(insertPosition, button);
          }
          ui->treeWidget->setItemWidget(item, 1, container);
        }

        _impl->graphDelegate->refreshData(tree, rows);

        ui->treeWidget->resizeColumnToContents(0);
      });

  auto newBranchAction = [=](const GitBranch &branch) {
    _impl->resetAction->setText(
        tr("Reset branch %1 to this commit...").arg(branch.name));
  };
  connect(newGitInterface.get(), &GitInterface::branchChanged,
          activeRepositoryContext.get(), newBranchAction);
  newBranchAction(newGitInterface->activeBranch());
}
