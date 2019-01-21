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
  virtual ~DiffView() override;
  virtual QVariant configuration() override;
  virtual void configure(const QVariant &configuration) override;

private:
  Ui::DiffView *ui;
  QScopedPointer<DiffViewPrivate> _impl;
};

#endif // DIFFVIEW_H