#include "treewidgetitem.hpp"


TreeWidgetItem::TreeWidgetItem(QObject* parent)
  : QObject(parent),
    QTreeWidgetItem()
{}

TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem* parentWidget, const QStringList& texts, QObject *parent)
  : QObject(parent),
    QTreeWidgetItem(parentWidget, texts)
{}

TreeWidgetItem::TreeWidgetItem(QTreeWidget* parentWidget, const QStringList& texts, QObject* parent)
  : QObject(parent),
    QTreeWidgetItem(parentWidget, texts)
{}

TreeWidgetItem::~TreeWidgetItem()
{
}
