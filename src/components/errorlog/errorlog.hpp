#ifndef ERRORLOG_HPP
#define ERRORLOG_HPP

#include "components/dockwidget.hpp"

namespace Ui {
class ErrorLog;
}

struct ErrorLogImpl;

class ErrorLog : public DockWidget {
  Q_OBJECT
  DOCK_WIDGET

  friend struct ErrorLogImpl;

public:
  explicit ErrorLog(MainWindow *mainWindow);
  ~ErrorLog();

  void configure(const QVariant &) override;
  void onError(const QString &message, ActionTag, ErrorType type) override;

private slots:
  void on_pushButton_clicked();

private:
  Ui::ErrorLog *ui;
  QScopedPointer<ErrorLogImpl> _impl;
};

#endif // ERRORLOG_HPP
