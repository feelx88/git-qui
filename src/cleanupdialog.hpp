#ifndef CLEANUPDIALOG_HPP
#define CLEANUPDIALOG_HPP

#include <QDialog>

namespace Ui {
class CleanUpDialog;
}

struct CleanUpDialogPrivate;
class Core;

class CleanUpDialog : public QDialog {
  Q_OBJECT

public:
  explicit CleanUpDialog(Core *core, QWidget *parent = nullptr);
  virtual ~CleanUpDialog();

private:
  Ui::CleanUpDialog *ui;
  QScopedPointer<CleanUpDialogPrivate> _impl;
};

#endif // CLEANUPDIALOG_HPP
