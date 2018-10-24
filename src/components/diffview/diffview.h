#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include "components/dockwidget.hpp"

struct DiffViewPrivate;

namespace Ui {
class DiffView;
}

class DiffView : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET
  friend struct DiffViewPrivate;

public:
  explicit DiffView(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface);
  ~DiffView();

private:
  Ui::DiffView *ui;
  QScopedPointer<DiffViewPrivate> _impl;
};

#endif // DIFFVIEW_H
