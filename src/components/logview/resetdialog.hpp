#ifndef RESETDIALOG_HPP
#define RESETDIALOG_HPP

#include <QDialog>

#include "gitinterface.hpp"

namespace Ui {
class ResetDialog;
}

struct ResetDialogPrivate;

class ResetDialog : public QDialog {
  Q_OBJECT

public:
  explicit ResetDialog(const GitBranch &ref, const GitCommit &commit,
                       QWidget *parent);
  ~ResetDialog();

  GitInterface::ResetType resetType();

private:
  Ui::ResetDialog *ui;
  QScopedPointer<ResetDialogPrivate> _impl;
};

#endif // RESETDIALOG_HPP
