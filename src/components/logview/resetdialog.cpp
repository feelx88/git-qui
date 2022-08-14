#include "resetdialog.hpp"
#include "gitbranch.hpp"
#include "gitcommit.hpp"
#include "ui_resetdialog.h"

struct ResetDialogPrivate {
  GitInterface::ResetType resetType;
};

ResetDialog::ResetDialog(const GitBranch &branch, const GitCommit &commit,
                         QWidget *parent)
    : QDialog(parent), ui(new Ui::ResetDialog), _impl(new ResetDialogPrivate) {
  ui->setupUi(this);
  ui->label->setText(ui->label->text().arg(branch.name, commit.id));
}

ResetDialog::~ResetDialog() { delete ui; }

GitInterface::ResetType ResetDialog::resetType() {
  if (ui->radioButton->isChecked()) {
    return GitInterface::ResetType::SOFT;
  }

  if (ui->radioButton_3->isChecked()) {
    return GitInterface::ResetType::HARD;
  }

  return GitInterface::ResetType::MIXED;
}
