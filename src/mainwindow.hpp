#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariantMap>

namespace Ui {
class MainWindow;
}

struct MainWindowPrivate;
class Core;
class GitInterface;
class Project;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  friend struct MainWindowPrivate;

public:
  explicit MainWindow(Core *core, const QVariantMap &configuration = QVariantMap());
  ~MainWindow();

  Core *core();

  QVariant configuration() const;

  QToolBar *addToolbar(Qt::ToolBarArea area);
  QMainWindow *createTab(const QString &title);

  void setEditMode(bool enabled);

protected:
  void changeEvent(QEvent *);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> _impl;
};

#endif // MAINWINDOW_H
