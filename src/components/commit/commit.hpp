#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <QDockWidget>

#include "components/dockwidget.hpp"

namespace Ui {
class Commit;
}

struct CommitPrivate;

class Commit : public DockWidget {
  Q_OBJECT
  DOCK_WIDGET
  friend struct CommitPrivate;

public:
  explicit Commit(MainWindow *mainWindow);
  virtual ~Commit() override;
  virtual QVariant configuration() override;
  virtual void configure(const QVariant &configuration) override;

protected:
  virtual QVariantMap getProjectSpecificConfiguration() override;

  virtual void onProjectSwitched(Project *newProject) override;
  virtual void onProjectSpecificConfigurationLoaded(
      const QVariantMap &configuration) override;
  virtual void onRepositorySwitched(GitInterface *newGitInterface,
                                    QObject *activeRepositoryContext) override;
  virtual void onError(const QString &message, ActionTag tag,
                       ErrorType type) override;

private:
  Ui::Commit *ui;
  QScopedPointer<CommitPrivate> _impl;
};

#endif // COMMIT_HPP
