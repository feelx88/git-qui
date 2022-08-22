#ifndef TOOLBARACTIONS_HPP
#define TOOLBARACTIONS_HPP

#include <QList>
#include <QMap>

class QAction;
class Core;

class ToolBarActions {
public:
  struct ActionID {
    static constexpr const char *STASH = "stash";
    static constexpr const char *UNSTASH = "unstash";
    static constexpr const char *FETCH = "fetch";
    static constexpr const char *PUSH = "push";
    static constexpr const char *PULL = "pull";
    static constexpr const char *PUSH_ALL = "push-all";
    static constexpr const char *PULL_ALL = "pull-all";
    static constexpr const char *NEW_BRANCH = "new-branch";
    static constexpr const char *CLEANUP = "cleanup";
  };

  struct ActionCallerProperty {
    static constexpr const char *NEW_BRANCH_BASE_COMMIT =
        "new-branch_base-commit";
  };

  static void initialize(Core *core);
  static const QMap<QString, QAction *> all();
  static QAction *byId(const QString &id);

private:
  static QMap<QString, QAction *> _actionMap;
  static void addAction(QString id, QString icon, QString text);
  static QWidget *focusedWidget(const QString &id);
};

#endif // TOOLBARACTIONS_HPP
