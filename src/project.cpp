#include "mainwindow.hpp"
#include "project.hpp"

#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <QFileDialog>

#include "gitinterface.hpp"

struct ProjectImpl
{
  static inline const QString CONFIG_NAME = "name";
  static inline const QString CONFIG_REPOSITORY_LIST = "repositoryList";
  static inline const QString CONFIG_CURRENT_REPOSITORY = "currentRepository";

  static inline const QString CONFIG_REPOSITORY_LIST_NAME = "name";
  static inline const QString CONFIG_REPOSITORY_LIST_PATH = "path";

  Project *_this;
  QSettings *settings = nullptr;
  QString name;
  QList<GitInterface*> repositories;
  int currentRepository = 0;
  QList<QRegularExpression> ignoredSubdirectories = {
    QRegularExpression("/.*vendor.*/"),
    QRegularExpression("/.*node_modules.*/")
  };

  ProjectImpl(Project *project)
    :_this(project)
  {}

  void loadSettings()
  {
    name = settings->value(CONFIG_NAME).toString();
    currentRepository = settings->value(CONFIG_CURRENT_REPOSITORY, 0).toInt();

    QList<QVariantMap> list = qvariant_cast<QList<QVariantMap>>(settings->value(CONFIG_REPOSITORY_LIST));

    for (auto entry : list)
    {
      repositories.append(new GitInterface(
        entry.value(CONFIG_REPOSITORY_LIST_NAME).toString(),
        entry.value(CONFIG_REPOSITORY_LIST_PATH).toString(),
        _this
      ));
    }
  }

  void writeSettings()
  {
    if (settings)
    {
      settings->setValue(CONFIG_NAME, name);
      settings->setValue(CONFIG_CURRENT_REPOSITORY, currentRepository);
      QList<QVariantMap> list;

      for (auto repository : repositories)
      {
        list << QVariantMap {
          {CONFIG_REPOSITORY_LIST_NAME, repository->name()},
          {CONFIG_REPOSITORY_LIST_PATH, repository->path()}
        };
      }

      settings->setValue(CONFIG_REPOSITORY_LIST, QVariant::fromValue(list));
      settings->sync();
    }
  }
};

Project::Project(const QString &fileName, QObject *parent)
  : Project(parent)
{
  setFileName(fileName);
}

Project::Project(QObject *parent)
: QObject(parent),
  _impl(new ProjectImpl(this))
{
}

QString Project::fileName() const
{
  return _impl->settings ? _impl->settings->fileName() : "";
}

QString Project::name() const
{
  return _impl->name;
}

QList<GitInterface*> Project::repositoryList() const
{
  return _impl->repositories;
}

GitInterface *Project::activeRepository()
{
  return _impl->repositories.at(_impl->currentRepository);
}

void Project::addRepository()
{
  QString path = QFileDialog::getExistingDirectory(
    nullptr,
    QObject::tr("Select repository path"),
    QDir::current().path()
  );

  auto list = repositoryList();

  if (!path.isNull())
  {
    QDirIterator iterator(
      path,
      {".git"},
      QDir::Dirs | QDir::Hidden,
      QDirIterator::Subdirectories
    );

    auto regex = QRegularExpression();
    while (iterator.hasNext())
    {
      QDir currentDir = QDir(iterator.next());
      currentDir.cdUp();

      bool directoryValid = true;

      for (auto regex : _impl->ignoredSubdirectories)
      {
        if (regex.match(currentDir.path()).hasMatch())
        {
          directoryValid = false;
          break;
        }
      }

      if(directoryValid)
      {
        _impl->repositories.append(new GitInterface(currentDir.dirName(), currentDir.absolutePath(), this));
      }
    }

    _impl->writeSettings();
  }

}

void Project::removeRepository(const int &index)
{
  _impl->repositories.removeAt(index);
  _impl->writeSettings();
}

void Project::setCurrentRepository(const int &index)
{
  _impl->currentRepository = index;
  emit repositorySwitched(_impl->repositories.at(index));
  _impl->repositories.at(index)->reload();
}

void Project::setCurrentRepository(GitInterface *repository)
{
  _impl->currentRepository = _impl->repositories.indexOf(repository);
  emit repositorySwitched(_impl->repositories.at(_impl->currentRepository));
  _impl->repositories.at(_impl->currentRepository)->reload();
}

void Project::setCurrentRepository(const QString &name)
{
  auto found = std::find_if(
    _impl->repositories.begin(),
    _impl->repositories.end(),
    [name](GitInterface *repository)
    {
      return repository->name() == name;
    }
  );

  if (found != _impl->repositories.end())
  {
    return;
  }

  _impl->currentRepository = _impl->repositories.indexOf(*found);
  emit repositorySwitched(_impl->repositories.at(_impl->currentRepository));
  _impl->repositories.at(_impl->currentRepository)->reload();
}

void Project::setName(const QString &name)
{
  _impl->name = name;
  _impl->writeSettings();
}

void Project::setFileName(const QString &fileName)
{
  if (!_impl->settings)
  {
    _impl->settings = new QSettings(fileName, QSettings::IniFormat, this);
    _impl->loadSettings();
    _impl->writeSettings();
  }
}

void Project::save()
{
  _impl->writeSettings();
}
