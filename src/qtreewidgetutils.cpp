#include "qtreewidgetutils.hpp"
#include "treewidgetitem.hpp"

void TreeWidgetUtils::createItems(QTreeWidget *parentWidget,
                                  const QList<QString> &itemLabels,
                                  const QString &separator) {
  for (const auto &itemLabel : itemLabels) {
    auto parentItem = parentWidget->invisibleRootItem();

    auto itemLabelsContainer = itemLabel.split(separator);
    for (const auto &part : itemLabelsContainer) {
      QTreeWidgetItem *currentItem = nullptr;

      for (int x = 0; x < parentItem->childCount(); ++x) {
        if (parentItem->child(x)->text(0) == part) {
          currentItem = parentItem->child(x);
          break;
        }
      }

      if (!currentItem) {
        currentItem = new TreeWidgetItem(parentItem, {part}, parentWidget);
      }

      parentItem = currentItem;
    }

    parentItem->setData(0, Qt::UserRole, itemLabel);
  }
}
