#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

struct MainWindowPrivate;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  friend struct MainWindowPrivate;

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> _impl;
};

#endif // MAINWINDOW_H
