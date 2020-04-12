#include "core.hpp"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "projectsettingsdialog.hpp"
#include "initialwindowconfiguration.hpp"

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

  void addWindow(const QVariant &configuration)
  {
    auto window = new MainWindow(_this, configuration.toMap());

    if (configuration.isNull())
    {
      InitialWindowConfiguration::create(window);
    }

    window->show();
    mainWindows.append(window);
  }
};

Core::Core(QObject *parent)
: QObject(parent),
  _impl(new CoreImpl(this))
{}

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
  QString projectFileName = settings.value(ConfigurationKey::CURRENT_PROJECT).toString();
  Project *project = nullptr;

  if (projectFileName.isEmpty())
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
      project = new Project(this);
      auto settingsDialog = new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::CREATE, project);
      if (settingsDialog->exec() != QDialog::Accepted)
      {
        QMessageBox::critical(nullptr, tr("Error"), tr("No project opened, closing."));
        return false;
      }
      break;
    }
    case QMessageBox::Open:
    {
      QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select project to open"));

      if (!fileName.isEmpty())
      {
        project = new Project(fileName, this);
      }
      break;
    }
    default:
      QMessageBox::critical(nullptr, tr("Error"), tr("No project opened, closing."));
      return false;
    }
  }
  else
  {
    project = new Project(projectFileName, this);
  }

  _impl->project = project;

  if (settings.contains(ConfigurationKey::MAIN_WINDOWS))
  {
    for (auto& windowConfiguration : settings.value(ConfigurationKey::MAIN_WINDOWS).toList())
    {
      _impl->addWindow(windowConfiguration);
    }
  }
  else
  {
    _impl->addWindow(QVariant());
  }

  _impl->project->activeRepository()->reload();

  return true;
}

void Core::changeProject(Project *project)
{
  _impl->project = project;

  QSettings settings;
  settings.setValue(ConfigurationKey::CURRENT_PROJECT, project->fileName());

  emit projectChanged(_impl->project);

  project->activeRepository()->reload();
}

Project *Core::project() const
{
  return _impl->project;
}
