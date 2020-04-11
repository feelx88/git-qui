#ifndef TOOLBARACTIONS_HPP
#define TOOLBARACTIONS_HPP

#include <QList>
#include <QMap>

class QAction;
class Core;

class ToolBarActions
{
public:
  static void initialize(Core *core);
  static const QMap<QString, QAction *> all();
  static QAction* byId(const QString &id);

private:
  static QMap<QString, QAction*> _actionMap;
  static void addAction(QString id, QString icon, QString text);
};

#endif // TOOLBARACTIONS_HPP
