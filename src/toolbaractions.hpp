#ifndef TOOLBARACTIONS_HPP
#define TOOLBARACTIONS_HPP

#include <QList>
#include <QMap>

class QAction;
class MainWindow;

class ToolBarActions
{
public:
  static void initialize(MainWindow *mainWindow);
  static const QMap<QString, QAction *> all();
  static QAction* byId(const QString &id);

private:
  static QMap<QString, QAction*> _actionMap;
};

#endif // TOOLBARACTIONS_HPP
