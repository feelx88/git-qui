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
  friend struct ProjectSettingsDialogImpl;

  enum class DialogMode {
    CREATE = 0,
    EDIT
  };

  explicit ProjectSettingsDialog(DialogMode dialogMode, Project *project, QWidget *parent = nullptr);
  ~ProjectSettingsDialog();

private:
  Ui::ProjectSettingsDialog *ui;
  ProjectSettingsDialogImpl *_impl;
};

#endif // PROJECTSETTINGSDIALOG_HPP
