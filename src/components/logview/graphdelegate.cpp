#include "graphdelegate.h"

#include <QPainter>

#include "gittree.hpp"

struct GraphDelegatePrivate {
  QSharedPointer<GitTree> gitTree;
  QList<GraphDelegate::RowInfo> rows;
  QMap<QString, int> commitColumns;
  int minWidth = 0;
};

GraphDelegate::GraphDelegate(QObject *parent)
    : QItemDelegate{parent}, _impl(new GraphDelegatePrivate) {
  setClipping(false);
}

void GraphDelegate::refreshData(QSharedPointer<GitTree> gitTree,
                                QList<RowInfo> rows) {
  _impl->gitTree = gitTree;
  _impl->rows = rows;

  int maxColumns = 1;

  for (int row = 0; row < _impl->rows.size(); ++row) {
    auto rowInfo = _impl->rows.at(row);
    _impl->commitColumns.insert(rowInfo.commitId, rowInfo.column);

    maxColumns = qMax(maxColumns, rowInfo.columnCount);
  }

  _impl->minWidth = (9 * 2) + (maxColumns * 24);
}

void GraphDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  QPoint center(option.rect.x() + 9, option.rect.center().y() + 1);
  QPoint halfHeight = QPoint(0, (option.rect.height() / 2) - 1);
  auto commit = _impl->gitTree->commitList().at(index.row());

  if (commit->id.isNull()) {
    return;
  }

  drawBackground(painter, option, index);

  RowInfo currentRow = _impl->rows.value(index.row());

  painter->setPen(QPen(QBrush(Qt::red), 2));

  for (const auto &childCommit : qAsConst(commit->childCommits)) {
    painter->drawLine(
        center + QPoint(24 * currentRow.column, 0),
        center + QPoint(24 * _impl->commitColumns.value(childCommit.lock()->id),
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

  painter->setBrush(commit->isHead ? Qt::green : Qt::blue);
  painter->setPen(commit->isHead ? Qt::green : Qt::blue);
  painter->drawEllipse(center + QPoint(currentRow.column * 24, 0), 6, 6);

  painter->restore();
}

QSize GraphDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &) const {
  return QSize(_impl->minWidth, option.rect.height());
}
