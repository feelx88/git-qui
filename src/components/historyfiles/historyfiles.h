#ifndef HISTORYFILES_H
#define HISTORYFILES_H

#include <components/dockwidget.hpp>

namespace Ui {
class HistoryFiles;
}

struct HistoryFilesPrivate;

class HistoryFiles : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET

public:
  friend class HistoryFilesPrivate;

  explicit HistoryFiles(MainWindow *mainWindow);
  ~HistoryFiles();

protected:
  void onRepositorySwitched(QSharedPointer<GitInterface> newGitInterface, QSharedPointer<QObject> activeRepositoryContext) override;

private:
  Ui::HistoryFiles *ui;
  QScopedPointer<HistoryFilesPrivate> _impl;
};

#endif // HISTORYFILES_H
