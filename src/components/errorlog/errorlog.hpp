#ifndef ERRORLOG_HPP
#define ERRORLOG_HPP

#include "components/dockwidget.hpp"

namespace Ui {
class ErrorLog;
}

class ErrorLog : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET

public:
  explicit ErrorLog(MainWindow *mainWindow);
  ~ErrorLog();

  void configure(const QVariant &) override;
  void onError(const QString &message, ErrorTag) override;

private slots:
  void on_pushButton_clicked();

private:
  Ui::ErrorLog *ui;
};

#endif // ERRORLOG_HPP
