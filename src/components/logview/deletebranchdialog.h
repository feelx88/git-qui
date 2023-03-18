#ifndef DELETEBRANCHDIALOG_H
#define DELETEBRANCHDIALOG_H

#include <QDialog>

namespace Ui {
class DeleteBranchDialog;
}

class DeleteBranchDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DeleteBranchDialog(const QString &branchName, QWidget *parent = nullptr);
  ~DeleteBranchDialog();

  bool forceDelete() const;

private:
  Ui::DeleteBranchDialog *ui;
};

#endif // DELETEBRANCHDIALOG_H
