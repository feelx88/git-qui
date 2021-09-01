#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include "components/dockwidget.hpp"

struct DiffViewPrivate;

namespace Ui {
class DiffView;
}

class DiffView : public DockWidget {
  Q_OBJECT
  DOCK_WIDGET
  friend struct DiffViewPrivate;

public:
  explicit DiffView(MainWindow *mainWindow);
  virtual ~DiffView() override;
  virtual QVariant configuration() override;
  virtual void configure(const QVariant &configuration) override;

protected:
  virtual void onProjectSwitched(Project *newProject) override;
  virtual void onRepositorySwitched(GitInterface *newGitInterface,
                                    QObject *activeRepositoryContext) override;

private:
  Ui::DiffView *ui;
  QScopedPointer<DiffViewPrivate> _impl;
};

#endif // DIFFVIEW_H
