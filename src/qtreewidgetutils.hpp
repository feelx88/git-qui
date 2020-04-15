#ifndef QTREEWIDGETUTILS_HPP
#define QTREEWIDGETUTILS_HPP

#include <QString>

class QObject;
class TreeWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;

namespace QTreeWidgetUtils
{
QList<QTreeWidgetItem*> createItems(QTreeWidget *parentWidget, const QList<QString> &itemLabels, const QString &separator = "/", QObject *parent = nullptr);
};

#endif // QTREEWIDGETUTILS_HPP
