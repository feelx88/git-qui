#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <QDockWidget>

#include "components/dockwidget.hpp"

namespace Ui {
class Commit;
}

struct CommitPrivate;

class Commit : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET
  friend struct CommitPrivate;

public:
  explicit Commit(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface);
  virtual ~Commit() override;
  virtual QVariant configuration() override;
  virtual void configure(const QVariant &configuration) override;

private:
  Ui::Commit *ui;
  QScopedPointer<CommitPrivate> _impl;
};

#endif // COMMIT_HPP
