#ifndef QTREEWIDGETUTILS_HPP
#define QTREEWIDGETUTILS_HPP

#include <QString>

class QObject;
class QTreeWidget;
class QTreeWidgetItem;

namespace TreeWidgetUtils {
void createItems(QTreeWidget *parentWidget,
                                     const QList<QString> &itemLabels,
                                     const QString &separator = "/");
};

#endif // QTREEWIDGETUTILS_HPP
