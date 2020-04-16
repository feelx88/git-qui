#include "qtreewidgetutils.hpp"
#include "treewidgetitem.hpp"

QList<QTreeWidgetItem*> TreeWidgetUtils::createItems(QTreeWidget *parentWidget, const QList<QString> &itemLabels, const QString &separator, QObject *parent)
{
  QList<QTreeWidgetItem*> items;
  QMap<QString, QTreeWidgetItem*> itemMap;

  for (auto label : itemLabels)
  {
    auto parts = label.split(separator);
    QTreeWidgetItem *rootItem = nullptr;
    for (auto item : items)
    {
      if (item->text(0) == parts[0])
      {
        rootItem = item;
        break;
      }
    }

    if (!rootItem)
    {
      rootItem = new TreeWidgetItem(parentWidget, {parts[0]}, parent);
    }

    QTreeWidgetItem *item = rootItem;

    for (int x = 1; x < parts.length(); ++x)
    {
      auto foundItem = itemMap.find(parts[x]);
      if (foundItem != itemMap.end())
      {
        item = foundItem.value();
      }
      else
      {
        item = new TreeWidgetItem(item, {parts[x]}, parent);
        itemMap.insert(parts[x], item);
      }
    }

    item->setData(0, Qt::UserRole, label);
    items.append(rootItem);
  }

  return items;
}
