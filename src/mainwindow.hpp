#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

struct MainWindowPrivate;
class GitInterface;
class Project;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  friend struct MainWindowPrivate;

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  const QList<GitInterface*> repositories() const;
  Project *project();

signals:
  void repositoryAdded(GitInterface *repository);
  void repositoryRemoved(GitInterface *repository);
  void repositorySwitched(GitInterface *repository);

protected:
  void changeEvent(QEvent *);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> _impl;
};

#endif // MAINWINDOW_H
