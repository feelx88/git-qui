#ifndef PROJECTSETTINGSDIALOG_HPP
#define PROJECTSETTINGSDIALOG_HPP

#include <QDialog>

namespace Ui {
class ProjectSettingsDialog;
}

struct ProjectSettingsDialogPrivate;

class Project;

class ProjectSettingsDialog : public QDialog {
  Q_OBJECT

public:
  friend struct ProjectSettingsDialogPrivate;

  enum class DialogMode { CREATE = 0, EDIT };

  explicit ProjectSettingsDialog(DialogMode dialogMode, Project *project,
                                 QWidget *parent = nullptr);
  ~ProjectSettingsDialog();

private:
  Ui::ProjectSettingsDialog *ui;
  QScopedPointer<ProjectSettingsDialogPrivate> _impl;
};

#endif // PROJECTSETTINGSDIALOG_HPP
