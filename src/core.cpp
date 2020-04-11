#include "core.hpp"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "projectsettingsdialog.hpp"

struct ConfigurationKey
{
  static constexpr const char *CURRENT_PROJECT = "currentProject";
  static constexpr const char *MAIN_WINDOWS = "mainWindows";
};

struct CoreImpl
{
  Core *_this;
  Project *project;
  QList<MainWindow*> mainWindows;

  CoreImpl(Core *core)
    : _this(core)
  {}
};

Core::Core(QObject *parent)
: QObject(parent),
  _impl(new CoreImpl(this))
{
}

Core::~Core()
{
  QSettings settings;

  QVariantList mainWindows;
  for (auto &window : _impl->mainWindows)
  {
    mainWindows.append(window->configuration());
  }
  settings.setValue(ConfigurationKey::MAIN_WINDOWS, mainWindows);

  settings.sync();
}

bool Core::init()
{
  QSettings settings;

  if (!settings.contains(ConfigurationKey::CURRENT_PROJECT))
  {
    QMessageBox dialog(
      QMessageBox::Question,
      tr("No Project selected"),
      tr("Would you like to create a new project? Alternatively, you could open an existing one."),
      QMessageBox::Yes | QMessageBox::Open | QMessageBox::Abort
    );
    dialog.setButtonText(QMessageBox::Yes, tr("Create new project"));
    dialog.setButtonText(QMessageBox::Open, tr("Open existing project"));

    switch (dialog.exec())
    {
    case QMessageBox::Yes:
    {
      Project *project = new Project(this);
      auto settingsDialog = new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::CREATE, project);
      if (settingsDialog->exec() == QDialog::Accepted)
      {
        _impl->project = project;
      }
      break;
    }
    case QMessageBox::Open:
    {
      QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select project to open"));

      if (!fileName.isEmpty())
      {
        _impl->project = new Project(fileName, this);
      }
      break;
    }
    default:
      QMessageBox::critical(nullptr, tr("Error"), tr("No project opened, closing."));
      return false;
    }
  }

  changeProject(settings.value(ConfigurationKey::CURRENT_PROJECT).toString());

  if (settings.contains(ConfigurationKey::MAIN_WINDOWS))
  {
    for (auto& windowConfiguration : settings.value(ConfigurationKey::MAIN_WINDOWS).toList())
    {
      _impl->mainWindows.append(new MainWindow(this, windowConfiguration.toMap()));
    }
  }
  else
  {
    auto window = new MainWindow(this);
    window->show();
    _impl->mainWindows.append(window);
  }

  return true;
}

void Core::changeProject(const QString &path)
{
  _impl->project = new Project(path, this);

  QSettings settings;
  settings.setValue(ConfigurationKey::CURRENT_PROJECT, path);

  emit projectChanged(_impl->project);
}

Project *Core::project() const
{
  return _impl->project;
}
