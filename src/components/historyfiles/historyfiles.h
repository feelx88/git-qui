#ifndef HISTORYFILES_H
#define HISTORYFILES_H

#include <components/dockwidget.hpp>

namespace Ui {
class HistoryFiles;
}

class HistoryFiles : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET

public:
  explicit HistoryFiles(MainWindow *mainWindow);
  ~HistoryFiles();

protected:
  void onRepositorySwitched(QSharedPointer<GitInterface> newGitInterface, QSharedPointer<QObject> activeRepositoryContext) override;

private:
  Ui::HistoryFiles *ui;
};

#endif // HISTORYFILES_H
