#ifndef BRANCHLIST_HPP
#define BRANCHLIST_HPP

#include "components/dockwidget.hpp"

namespace Ui {
class BranchList;
}

struct BranchListPrivate;

class BranchList : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET
  friend struct BranchListPrivate;

public:
  explicit BranchList(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface);
  virtual ~BranchList() override;

private:
  Ui::BranchList *ui;
  QScopedPointer<BranchListPrivate> _impl;
};

#endif // BRANCHLIST_HPP
