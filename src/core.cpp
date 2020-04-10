#include "core.hpp"

#include <QSharedPointer>
#include <QSettings>

#include "project.hpp"
#include "configurationkey.hpp"
#include "mainwindow.hpp"

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
  QSettings settings;
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
    _impl->mainWindows.append(window);
  }
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
