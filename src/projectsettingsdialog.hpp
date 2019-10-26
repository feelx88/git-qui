#ifndef PROJECTSETTINGSDIALOG_HPP
#define PROJECTSETTINGSDIALOG_HPP

#include <QDialog>

namespace Ui {
class ProjectSettingsDialog;
}

struct ProjectSettingsDialogImpl;

class Project;

class ProjectSettingsDialog : public QDialog
{
  Q_OBJECT

public:
  enum class DialogMode {
    CREATE = 0,
    EDIT
  };

  explicit ProjectSettingsDialog(DialogMode dialogMode, QWidget *parent = nullptr);
  ~ProjectSettingsDialog();

  Project *project();

private:
  Ui::ProjectSettingsDialog *ui;
  ProjectSettingsDialogImpl *_impl;
};

#endif // PROJECTSETTINGSDIALOG_HPP
