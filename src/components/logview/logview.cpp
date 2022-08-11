#include "logview.hpp"
#include "ui_logview.h"

#include "gitcommit.hpp"
#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "treewidgetitem.hpp"

#include <QItemDelegate>
#include <QPainter>
#include <iostream>

struct RowInfo {
  int column = 0, columnCount = 1;
  QString commitId;
  QMultiMap<QString, int> currentColumns;
  QMultiMap<QString, int> childColumns;
};

struct Delegate : public QItemDelegate {
  QSharedPointer<GitTree> gitTree;
  QList<RowInfo> rows;
  QMap<QString, int> commitColumns;

  Delegate(QObject *parent) : QItemDelegate(parent) { setClipping(false); }

  void refreshData(QSharedPointer<GitTree> gitTree, QList<RowInfo> rows) {
    this->gitTree = gitTree;
    this->rows = rows;

    for (int row = 0; row < this->rows.size(); ++row) {
      auto rowInfo = this->rows.at(row);
      commitColumns.insert(rowInfo.commitId, rowInfo.column);
    }
  }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPoint center(option.rect.x() + 9, option.rect.center().y() + 1);
    QPoint halfHeight = QPoint(0, (option.rect.height() / 2) - 1);
    auto commit = gitTree->commitList().at(index.row());

    drawBackground(painter, option, index);

    RowInfo currentRow = rows.value(index.row());
    painter->setPen(QPen(QBrush(Qt::red), 2));

    for (const auto &childCommit : qAsConst(commit->childCommits)) {
      painter->drawLine(
          center + QPoint(24 * currentRow.column, 0),
          center + QPoint(24 * commitColumns.value(childCommit.lock()->id),
                          -halfHeight.y()));
    }

    auto filledChildColumns = currentRow.childColumns.values();
    auto filledCurrentColumns = currentRow.currentColumns.values();

    for (int column = 0; column < currentRow.columnCount; ++column) {
      if (filledChildColumns.contains(column) &&
          filledCurrentColumns.contains(column)) {
        painter->drawLine(center + QPoint(column * 24, 0) - halfHeight,
                          center + QPoint(column * 24, 0));
      }

      if (filledCurrentColumns.contains(column)) {
        painter->drawLine(center + QPoint(column * 24, 0),
                          center + QPoint(column * 24, 0) + halfHeight);
      }
    }

    painter->setBrush(Qt::blue);
    painter->setPen(Qt::blue);
    painter->drawEllipse(center + QPoint(currentRow.column * 24, 0), 6, 6);

    painter->restore();
  }

  QSize sizeHint(const QStyleOptionViewItem &,
                 const QModelIndex &index) const override {
    return QSize(index.data(Qt::UserRole + 1).toList().size() * 5 + 10, 0);
  }
};

struct LogViewPrivate {
  GitInterface *gitInterface = nullptr;
  Delegate *graphDelegate;
  QList<RowInfo> rows;

  LogViewPrivate(LogView *_this) : graphDelegate(new Delegate(_this)) {}

  void connectSignals(LogView *_this) {
    _this->ui->treeWidget->setHeaderLabels({
        _this->tr("Graph"),
        _this->tr("Id"),
        _this->tr("Branches"),
        _this->tr("Message"),
        _this->tr("Author"),
        _this->tr("Date"),
    });
  }
};

DOCK_WIDGET_IMPL(LogView, tr("Log view"))

LogView::LogView(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::LogView),
      _impl(new LogViewPrivate(this)) {
  ui->setupUi(this);
  ui->treeWidget->setItemDelegateForColumn(0, _impl->graphDelegate);

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
        _impl->rows.clear();
        for (auto it = tree->commitList().rbegin();
             it != tree->commitList().rend(); ++it) {

          auto commit = *it;
          auto currentRow =
              _impl->rows.isEmpty() ? RowInfo() : RowInfo(_impl->rows.first());
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

          _impl->rows.prepend(currentRow);
        }

        for (const auto &commit : tree->commitList()) {
          TreeWidgetItem *item = new TreeWidgetItem(ui->treeWidget);
          item->setText(1, commit->id);
          item->setText(2, commit->branchHeads.join(", "));
          item->setText(3, commit->message);
          item->setText(4, commit->author);
          item->setText(5, commit->date.toString());

          ui->treeWidget->addTopLevelItem(item);
        }

        _impl->graphDelegate->refreshData(tree, _impl->rows);

        ui->treeWidget->resizeColumnToContents(1);
        ui->treeWidget->resizeColumnToContents(3);
        ui->treeWidget->resizeColumnToContents(4);
      });
}
