#ifndef TOOLBARACTIONS_HPP
#define TOOLBARACTIONS_HPP

#include <QList>
#include <QMap>

class QAction;
class Core;

class ToolBarActions
{
public:
  struct ActionID
  {
    static constexpr const char *STASH ="stash";
    static constexpr const char *UNSTASH = "unstash";
    static constexpr const char *PUSH = "push";
    static constexpr const char *PULL = "pull";
    static constexpr const char *PUSH_ALL = "push-all";
    static constexpr const char *PULL_ALL = "pull-all";
    static constexpr const char *NEW_BRANCH = "new-branch";
    static constexpr const char *CLEANUP = "cleanup";
  };

  static void initialize(Core *core);
  static const QMap<QString, QAction *> all();
  static QAction* byId(const QString &id);

private:
  static QMap<QString, QAction*> _actionMap;
  static void addAction(QString id, QString icon, QString text);
};

#endif // TOOLBARACTIONS_HPP
