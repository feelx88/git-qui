#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

struct MainWindowPrivate;
class GitInterface;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  friend struct MainWindowPrivate;

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void openRepository();
  void closeCurrentRepository();
  void switchRepository(const QString &path);

signals:
  void repositoryAdded(QSharedPointer<GitInterface> repository);
  void repositoryRemoved(QSharedPointer<GitInterface> repository);
  void repositorySwitched(QSharedPointer<GitInterface> repository);

protected:
  void changeEvent(QEvent *);

private:
  Ui::MainWindow *ui;

  QScopedPointer<MainWindowPrivate> _impl;
};

#endif // MAINWINDOW_H
