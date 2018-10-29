#ifndef LOGVIEW_HPP
#define LOGVIEW_HPP

#include "components/dockwidget.hpp"

namespace Ui {
class LogView;
}

struct LogViewPrivate;

class LogView : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET
  friend struct LogViewPrivate;

public:
  explicit LogView(QWidget *parent, QSharedPointer<GitInterface> gitInterface);
  ~LogView();
  QVariant configuration();

private:
  Ui::LogView *ui;
  QScopedPointer<LogViewPrivate> _impl;
};

#endif // LOGVIEW_HPP
