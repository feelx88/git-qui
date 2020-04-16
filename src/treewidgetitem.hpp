#ifndef TREEWIDGETITEM_HPP
#define TREEWIDGETITEM_HPP

#include <QObject>
#include <QTreeWidgetItem>

class TreeWidgetItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT
public:
  TreeWidgetItem(QObject *parent = nullptr);
  TreeWidgetItem(QTreeWidget* parentWidget, QObject *parent = nullptr);
  TreeWidgetItem(QTreeWidgetItem* parentWidget, QObject *parent = nullptr);
  TreeWidgetItem(QTreeWidgetItem* parentWidget, const QStringList& texts, QObject *parent = nullptr);
  TreeWidgetItem(QTreeWidget* parentWidget, const QStringList& texts, QObject *parent = nullptr);
  virtual ~TreeWidgetItem();
};

#endif // TREEWIDGETITEM_HPP
