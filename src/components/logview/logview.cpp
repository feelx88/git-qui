#include "logview.hpp"
#include "ui_logview.h"

#include "gitcommit.hpp"
#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "treewidgetitem.hpp"

#include <QItemDelegate>
#include <QPainter>
#include <iostream>

struct LaneInfo {
  int lane = 0, laneCount = 1;
};

struct Delegate : public QItemDelegate {
  QSharedPointer<GitTree> gitTree;
  QMap<QString, LaneInfo> lanes;

  Delegate(QObject *parent) : QItemDelegate(parent) {}

  void setGitTree(QSharedPointer<GitTree> gitTree) { this->gitTree = gitTree; }
  void setLanes(QMap<QString, LaneInfo> lanes) { this->lanes = lanes; }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    QPoint center(option.rect.x() + 9, option.rect.center().y());
    auto commit = gitTree->commitList().at(index.row());

    drawBackground(painter, option, index);

    for (int parentIndex = 0; parentIndex < commit->parentCommits.size();
         ++parentIndex) {
      painter->setPen(QPen(QBrush(Qt::red), 2));
      painter->drawLine(
          center, center + QPoint(parentIndex * 24, option.rect.height() / 2));
    }

    for (int childIndex = 0; childIndex < commit->childCommits.size();
         ++childIndex) {
      painter->setPen(QPen(QBrush(Qt::red), 2));
      painter->drawLine(
          center, center - QPoint(childIndex * -24, option.rect.height() / 2));
    }

    LaneInfo laneInfo = lanes.value(commit->id);

    for (int lane = 0; lane < laneInfo.laneCount; ++lane) {
      painter->setPen(QPen(QBrush(Qt::red), 2));
      painter->drawLine(
          center + QPoint(lane * 24, 0) - QPoint(0, option.rect.height() / 2),
          center + QPoint(lane * 24, 0) + QPoint(0, option.rect.height() / 2));

      if (lane == laneInfo.lane) {
        painter->setBrush(Qt::blue);
        painter->setPen(Qt::blue);
        painter->drawEllipse(center + QPoint(lane * 24, 0), 6, 6);
      }
    }
  }

  QSize sizeHint(const QStyleOptionViewItem &,
                 const QModelIndex &index) const override {
    return QSize(index.data(Qt::UserRole + 1).toList().size() * 5 + 10, 0);
  }
};

struct LogViewPrivate {
  GitInterface *gitInterface = nullptr;
  Delegate *graphDelegate;
  QMap<QString, LaneInfo> lanes;

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
  connect(newGitInterface, &GitInterface::logChanged, activeRepositoryContext,
          [=](QSharedPointer<GitTree> tree) {
            ui->treeWidget->clear();

            for (const auto &commit : tree->commitList()) {
              TreeWidgetItem *item = new TreeWidgetItem(ui->treeWidget);
              item->setText(1, commit->id);
              item->setText(2, commit->branchHeads.join(", "));
              item->setText(3, commit->message);
              item->setText(4, commit->author);
              item->setText(5, commit->date.toString());

              ui->treeWidget->addTopLevelItem(item);

              LaneInfo laneInfo = _impl->lanes.value(commit->id),
                       parentLaneInfo;

              if (commit->childCommits.size() > 0) {
                laneInfo.lane -= (commit->childCommits.size() - 1);
                laneInfo.laneCount -= (commit->childCommits.size() - 1);
              }

              laneInfo.lane = laneInfo.lane >= 0 ? laneInfo.lane : 0;
              laneInfo.laneCount =
                  laneInfo.laneCount >= 1 ? laneInfo.laneCount : 1;

              for (int parentIndex = 0;
                   parentIndex < commit->parentCommits.size(); ++parentIndex) {
                parentLaneInfo.lane = parentLaneInfo.lane + parentIndex;
                parentLaneInfo.laneCount =
                    laneInfo.laneCount + (commit->parentCommits.size() - 1);
                _impl->lanes.insert(
                    commit->parentCommits.at(parentIndex).lock()->id,
                    parentLaneInfo);
              }

              _impl->lanes.insert(commit->id, laneInfo);
            }

            _impl->graphDelegate->setGitTree(tree);
            _impl->graphDelegate->setLanes(_impl->lanes);

            ui->treeWidget->resizeColumnToContents(1);
            ui->treeWidget->resizeColumnToContents(3);
            ui->treeWidget->resizeColumnToContents(4);
          });
}
