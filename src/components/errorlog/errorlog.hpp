#ifndef ERRORLOG_HPP
#define ERRORLOG_HPP

#include "components/dockwidget.hpp"

namespace Ui {
class ErrorLog;
}

struct ErrorLogPrivate;

class ErrorLog : public DockWidget {
  Q_OBJECT
  DOCK_WIDGET

  friend struct ErrorLogPrivate;

public:
  explicit ErrorLog(MainWindow *mainWindow);
  ~ErrorLog();

  void configure(const QVariant &) override;
  void onError(const QString &message, GitInterface::ActionTag,
               GitInterface::ErrorType type, bool, QVariantMap) override;

private slots:
  void onPushButtonClicked();

private:
  Ui::ErrorLog *ui;
  QScopedPointer<ErrorLogPrivate> _impl;
};

#endif // ERRORLOG_HPP
