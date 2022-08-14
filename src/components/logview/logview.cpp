#include "logview.hpp"
#include "ui_logview.h"

#include "gitcommit.hpp"
#include "gitinterface.hpp"
#include "graphdelegate.h"
#include "mainwindow.hpp"
#include "treewidgetitem.hpp"

#include <QAction>
#include <QItemDelegate>
#include <QMenu>
#include <QPainter>
#include <QPushButton>

struct LogViewPrivate {
  GitInterface *gitInterface = nullptr;
  GraphDelegate *graphDelegate;
  QAction *resetAction;
  QMenu *branchMenu;

  LogViewPrivate(LogView *_this) : graphDelegate(new GraphDelegate(_this)) {
    branchMenu = new QMenu(_this);
    QObject::connect(branchMenu->addAction(QObject::tr("Checkout branch")),
                     &QAction::triggered, branchMenu, [=] {
                       gitInterface->changeBranch(
                           branchMenu->property("branch").toString());
                     });
  }

  void connectSignals(LogView *_this) {
    _this->ui->treeWidget->setHeaderLabels({
        _this->tr("Graph"),
        _this->tr("Refs"),
        _this->tr("Message"),
        _this->tr("Author"),
        _this->tr("Date"),
        _this->tr("Id"),
    });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    resetAction = new QAction(_this);
    QObject::connect(resetAction, &QAction::triggered, _this, [=] {
      GitCommit commit =
          _this->ui->treeWidget->currentItem()->data(0, 0).value<GitCommit>();
      gitInterface->resetToCommit(commit.id);
    });
    _this->ui->treeWidget->addAction(resetAction);
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

void LogView::onRepositorySwitched(GitInterface *newGitInterface,
                                   QObject *activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);
  _impl->gitInterface = newGitInterface;
  connect(
      newGitInterface, &GitInterface::logChanged, activeRepositoryContext,
      [=](QSharedPointer<GitTree> tree) {
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
          item->setText(3, commit->author);
          item->setText(4, commit->date.toString());
          item->setText(5, commit->id);

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
              button->setStyleSheet(
                  "QPushButton {color: black; background-color: "
                  "yellow; padding: 0.1em;}");
            } else if (ref.isRemote) {
              button->setStyleSheet(
                  "QPushButton {color: black; background-color: "
                  "gray; padding: 0.1em;}");
            } else if (ref.isHead) {
              insertPosition = 0;
              button->setStyleSheet(
                  "QPushButton {color: black; background-color: "
                  "lightgray; padding: 0.1em; "
                  "font-weight: bold}");
            } else {
              insertPosition = remoteIndex;
              button->setStyleSheet(
                  "QPushButton {color: black; background-color: "
                  "lightgray; padding: 0.1em;}");
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
  connect(newGitInterface, &GitInterface::branchChanged, this, newBranchAction);
  newBranchAction(newGitInterface->activeBranch());
}
