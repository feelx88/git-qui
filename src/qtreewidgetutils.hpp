#ifndef QTREEWIDGETUTILS_HPP
#define QTREEWIDGETUTILS_HPP

#include <QString>

class QObject;
class TreeWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;

struct QTreeWidgetUtils
{
  static QList<QTreeWidgetItem*> createItems(QTreeWidget *parentWidget, const QList<QString> &itemLabels, const QString &separator = "/", QObject *parent = nullptr);
};

#endif // QTREEWIDGETUTILS_HPP
