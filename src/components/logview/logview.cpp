#include "logview.hpp"
#include "ui_logview.h"

#include "deletebranchdialog.h"
#include "gitcommit.hpp"
#include "gitinterface.hpp"
#include "graphdelegate.h"
#include "mainwindow.hpp"
#include "toolbaractions.hpp"
#include "treewidgetitem.hpp"

#include <QAction>
#include <QClipboard>
#include <QInputDialog>
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
  QAction *resetAction, *checkoutAction, *deleteAction, *deleteTagAction,
      *cherryPickAction;
  QMenu *branchMenu, *tagMenu;

  LogViewPrivate(LogView *_this) : graphDelegate(new GraphDelegate(_this)) {}

  void connectSignals(LogView *_this) {
    _this->ui->treeWidget->setHeaderLabels({
        LogView::tr("Graph"),
        LogView::tr("Refs"),
        LogView::tr("Message"),
        LogView::tr("Author"),
        LogView::tr("Date"),
        LogView::tr("Id"),
    });

    QObject::connect(
        _this->ui->treeWidget, &QTreeWidget::currentItemChanged, _this,
        [=, this](QTreeWidgetItem *item) {
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

            gitInterface->historyStatus(commit.id);

            cherryPickAction->setText(
                LogView::tr("Cherry pick commit into branch %1")
                    .arg(gitInterface->activeBranch().name));
          }
        });

    _this->ui->treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    resetAction = new QAction("");
    _this->ui->treeWidget->addAction(resetAction);
    ToolBarActions::connectById(ToolBarActions::ActionID::RESET, resetAction);

    _this->ui->treeWidget->addAction(
        ToolBarActions::byId(ToolBarActions::ActionID::NEW_BRANCH));

    auto newTagAction = new QAction(LogView::tr("Create new tag"));
    QObject::connect(newTagAction, &QAction::triggered, _this, [=, this] {
      auto commitId = _this->ui->treeWidget->currentItem()
                          ->data(5, Qt::DisplayRole)
                          .toString();
      auto name = QInputDialog::getText(QApplication::activeWindow(),
                                        LogView::tr("Create new tag"),
                                        LogView::tr("New tag name"));

      if (!name.isEmpty() && !commitId.isEmpty()) {
        gitInterface->createTag(name, commitId);
      }
    });
    _this->ui->treeWidget->addAction(newTagAction);

    auto copyIdAction = new QAction(LogView::tr("Copy commit id"), _this);
    QObject::connect(copyIdAction, &QAction::triggered, _this, [=, this] {
      QGuiApplication::clipboard()->setText(_this->ui->treeWidget->currentItem()
                                                ->data(5, Qt::DisplayRole)
                                                .toString());
    });
    _this->ui->treeWidget->addAction(copyIdAction);

    cherryPickAction = new QAction("", _this);
    QObject::connect(cherryPickAction, &QAction::triggered, _this, [=, this] {
      auto commitId = _this->ui->treeWidget->currentItem()
                          ->data(5, Qt::DisplayRole)
                          .toString();
      gitInterface->cherryPickCommit(commitId);
    });
    _this->ui->treeWidget->addAction(cherryPickAction);

    branchMenu = new QMenu(_this);
    checkoutAction = branchMenu->addAction("");
    QObject::connect(checkoutAction, &QAction::triggered, branchMenu,
                     [=, this] {
                       gitInterface->changeBranch(
                           branchMenu->property("branch").value<GitRef>().name);
                     });

    deleteAction = branchMenu->addAction("");
    QObject::connect(deleteAction, &QAction::triggered, branchMenu, [=, this] {
      auto branch = branchMenu->property("branch").value<GitRef>().name;
      DeleteBranchDialog dialog(branch);
      if (dialog.exec() == QDialog::DialogCode::Accepted) {
        gitInterface->deleteBranch(branch, dialog.forceDelete());
      }
    });

    tagMenu = new QMenu(_this);

    auto copyTagAction = new QAction(LogView::tr("Copy tag name"), _this);
    QObject::connect(copyTagAction, &QAction::triggered, _this, [=, this] {
      QGuiApplication::clipboard()->setText(
          tagMenu->property("tag").value<GitRef>().name);
    });
    tagMenu->addAction(copyTagAction);

    deleteTagAction = tagMenu->addAction("");
    QObject::connect(deleteTagAction, &QAction::triggered, tagMenu, [=, this] {
      auto tag = tagMenu->property("tag").value<GitRef>().name;
      if (QMessageBox::question(_this, LogView::tr("Delete tag"),
                                LogView::tr("Delete tag %1?").arg(tag)) ==
          QMessageBox::Yes) {
        gitInterface->deleteTag(tag);
      }
    });
  }
};

DOCK_WIDGET_IMPL(LogView, LogView::tr("Log view"))

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
      activeRepositoryContext.get(), [=, this](QSharedPointer<GitTree> tree) {
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
                    [=, this](const QPoint &at) {
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
                      } else if (ref.isTag) {
                        _impl->tagMenu->setProperty("tag",
                                                    QVariant::fromValue(ref));
                        _impl->deleteTagAction->setText(
                            tr("Delete tag %1...").arg(ref.name));
                        _impl->tagMenu->popup(button->mapToGlobal(at));
                      }
                    });
            layout->insertWidget(insertPosition, button);
          }
          ui->treeWidget->setItemWidget(item, 1, container);
        }

        _impl->graphDelegate->refreshData(tree, rows);

        ui->treeWidget->resizeColumnToContents(0);
      });

  auto newBranchAction = [=, this](const GitBranch &branch) {
    _impl->resetAction->setText(
        tr("Reset branch %1 to this commit...").arg(branch.name));
  };
  connect(newGitInterface.get(), &GitInterface::branchChanged,
          activeRepositoryContext.get(), newBranchAction);
  newBranchAction(newGitInterface->activeBranch());
}

void LogView::onError(const QString &message, GitInterface::ActionTag actionTag,
                      GitInterface::ErrorType type, bool, QVariantMap context) {
  switch (actionTag) {
  case GitInterface::ActionTag::GIT_DELETE_BRANCH: {
    QMessageBox::warning(this, tr("Error while deleting a branch"), message);
    break;
  }
  case GitInterface::ActionTag::GIT_CHERRY_PICK: {
    bool ok = false;
    auto result = QInputDialog::getInt(
        this, tr("Error while cherry-picking a commit"),
        tr("%1\nTo repeat the cherry-pick with the --mainline "
           "flag to select the parent commit to use and press ok.\nUsually, "
           "parent #1 points "
           "to the main branch on merge commits")
            .arg(message),
        1, 1, 2147483647, 1, &ok);
    if (ok) {
      _impl->gitInterface->cherryPickCommit(
          context.value("commitId").toString(), result);
    }
    break;
  }
  default:
    break;
  }
}
