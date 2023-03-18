#include "deletebranchdialog.h"
#include "ui_deletebranchdialog.h"

#include <QPushButton>

DeleteBranchDialog::DeleteBranchDialog(const QString &branchName,
                                       QWidget *parent)
    : QDialog(parent), ui(new Ui::DeleteBranchDialog) {
  ui->setupUi(this);
  setWindowTitle(tr("Delete branch"));
  ui->label->setText(tr("Delete branch %1?").arg(branchName));

  connect(ui->buttonBox->button(QDialogButtonBox::Yes), &QPushButton::clicked,
          this, &DeleteBranchDialog::accept);
  connect(ui->buttonBox->button(QDialogButtonBox::No), &QPushButton::clicked,
          this, &DeleteBranchDialog::reject);
}

DeleteBranchDialog::~DeleteBranchDialog() { delete ui; }

bool DeleteBranchDialog::forceDelete() const {
  return ui->checkBox->isChecked();
}
