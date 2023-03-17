#ifndef LOGVIEW_HPP
#define LOGVIEW_HPP

#include "components/dockwidget.hpp"
#include <QEvent>
#include <QMetaEnum>

namespace Ui {
class LogView;
}

struct LogViewPrivate;

class LogView : public DockWidget {
  Q_OBJECT
  DOCK_WIDGET
  friend struct LogViewPrivate;

public:
  explicit LogView(MainWindow *mainWindow);
  virtual ~LogView() override;
  virtual QVariant configuration() override;
  virtual void configure(const QVariant &configuration) override;

protected:
  virtual void onProjectSwitched(Project *newProject) override;
  virtual void onRepositorySwitched(
      QSharedPointer<GitInterface> newGitInterface,
      QSharedPointer<QObject> activeRepositoryContext) override;
  void onError(const QString &message, GitInterface::ActionTag actionTag,
               GitInterface::ErrorType type) override;

private:
  Ui::LogView *ui;
  QScopedPointer<LogViewPrivate> _impl;
};

#endif // LOGVIEW_HPP
