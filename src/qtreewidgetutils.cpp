#include "qtreewidgetutils.hpp"

QList<QTreeWidgetItem*> QTreeWidgetUtils::createItems(QTreeWidget *parent, const QList<QString> &itemLabels, const QString &separator)
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
      rootItem = new QTreeWidgetItem(parent, {parts[0]});
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
        item = new QTreeWidgetItem(item, {parts[x]});
        itemMap.insert(parts[x], item);
      }
    }

    item->setData(0, Qt::UserRole, label);
    items.append(rootItem);
  }

  return items;
}
