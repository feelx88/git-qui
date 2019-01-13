#ifndef QTREEWIDGETUTILS_HPP
#define QTREEWIDGETUTILS_HPP

#include <QTreeWidgetItem>

struct QTreeWidgetUtils
{
  static QList<QTreeWidgetItem*> createItems(QTreeWidget *parent, const QList<QString> &itemLabels, const QString &separator = "/");
};

#endif // QTREEWIDGETUTILS_HPP
