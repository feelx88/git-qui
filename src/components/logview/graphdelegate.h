#ifndef GRAPHDELEGATE_H
#define GRAPHDELEGATE_H

#include <QItemDelegate>

struct GraphDelegatePrivate;
class GitTree;

class GraphDelegate : public QItemDelegate {
public:
  struct RowInfo {
    int column = 0, columnCount = 1;
    QString commitId;
    QMultiMap<QString, int> currentColumns;
    QMultiMap<QString, int> childColumns;
  };

  GraphDelegate(QObject *parent);

  void refreshData(QSharedPointer<GitTree> gitTree, QList<RowInfo> rows);

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &) const override;

private:
  GraphDelegatePrivate *_impl;
};

#endif // GRAPHDELEGATE_H
