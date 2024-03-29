#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUuid>
#include <QVariantMap>

#include <DockManager.h>

namespace Ui {
class MainWindow;
}

struct MainWindowPrivate;
class Core;
class GitInterface;
class Project;
class DockWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

  friend struct MainWindowPrivate;

public:
  explicit MainWindow(Core *core,
                      const QVariantMap &configuration = QVariantMap());
  ~MainWindow();

  Core *core();

  QVariant configuration() const;

  template <class T>
  DockWidget *
  addDockWidget(int tabIndex = -1, const QVariant &configuration = {},
                ads::DockWidgetArea area = ads::TopDockWidgetArea,
                const QString &uuid = QUuid::createUuid().toString()) {
    return addDockWidget(T::staticMetaObject.className(), tabIndex,
                         configuration, area, uuid);
  }
  QToolBar *addToolbar(Qt::ToolBarArea area);
  QMainWindow *createTab(const QString &title);

  void setEditMode(bool enabled);

protected:
  bool event(QEvent *);

private:
  DockWidget *
  addDockWidget(const QString &className, int tabIndex = -1,
                const QVariant &configuration = {},
                ads::DockWidgetArea area = ads::TopDockWidgetArea,
                const QString &uuid = QUuid::createUuid().toString());

  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> _impl;
};

#endif // MAINWINDOW_H
