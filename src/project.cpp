#include "mainwindow.hpp"
#include "project.hpp"

#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <QFileDialog>
#include <algorithm>

#include "gitinterface.hpp"

struct ConfigurationKeys
{
  static constexpr const char* NAME = "name";
  static constexpr const char* REPOSITORY_LIST = "repositoryList";
  static constexpr const char* CURRENT_REPOSITORY = "currentRepository";
  static constexpr const char* REPOSITORY_LIST_NAME = "name";
  static constexpr const char* REPOSITORY_LIST_PATH = "path";
};

struct ProjectPrivate
{
  Project *_this;
  QSettings *settings = nullptr;
  QString name;
  QList<GitInterface*> repositories;
  int currentRepository = 0;
  QList<QRegularExpression> ignoredSubdirectories = {
    QRegularExpression("/.*vendor.*/"),
    QRegularExpression("/.*node_modules.*/")
  };
  QScopedPointer<QObject> currentRepositoryContext = QScopedPointer<QObject>(new QObject(_this));

  ProjectPrivate(Project *project)
    :_this(project)
  {}

  void loadSettings()
  {
    name = settings->value(ConfigurationKeys::NAME).toString();
    QList<QVariantMap> list = qvariant_cast<QList<QVariantMap>>(settings->value(ConfigurationKeys::REPOSITORY_LIST));
    currentRepository = std::min(
      settings->value(ConfigurationKeys::CURRENT_REPOSITORY, 0).toInt(),
      list.size() - 1
    );


    for (auto entry : list)
    {
      repositories.append(new GitInterface(
        entry.value(ConfigurationKeys::REPOSITORY_LIST_NAME).toString(),
        entry.value(ConfigurationKeys::REPOSITORY_LIST_PATH).toString(),
        _this
      ));
    }
  }

  void writeSettings()
  {
    if (settings)
    {
      settings->setValue(ConfigurationKeys::NAME, name);

      currentRepository = currentRepository > repositories.size() ? 0 : currentRepository;

      settings->setValue(ConfigurationKeys::CURRENT_REPOSITORY, currentRepository);
      QList<QVariantMap> list;

      for (auto repository : repositories)
      {
        list << QVariantMap {
          {ConfigurationKeys::REPOSITORY_LIST_NAME, repository->name()},
          {ConfigurationKeys::REPOSITORY_LIST_PATH, repository->path()}
        };
      }

      settings->setValue(ConfigurationKeys::REPOSITORY_LIST, QVariant::fromValue(list));
      settings->sync();
    }
  }

  void changeRepository()
  {
    currentRepositoryContext.reset(new QObject(_this));
    auto repo = repositories.at(currentRepository);
    emit _this->repositorySwitched(repo, currentRepositoryContext.get());
    repo->reload();
  }
};

Project::Project(const QString &fileName, QObject *parent)
  : Project(parent)
{
  setFileName(fileName);
}

Project::Project(QObject *parent)
: QObject(parent),
  _impl(new ProjectPrivate(this))
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

GitInterface *Project::activeRepository() const
{
  return _impl->repositories.at(_impl->currentRepository);
}

QObject* Project::activeRepositoryContext() const
{
  return _impl->currentRepositoryContext.get();
}

GitInterface* Project::repositoryByName(const QString& name) const
{
  auto it = std::find_if(_impl->repositories.begin(), _impl->repositories.end(), [=](GitInterface *repository){
    return repository->name() == name;
  });

  if (it != _impl->repositories.end())
  {
    return *it;
  }
  return nullptr;
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
  _impl->changeRepository();
}

void Project::setCurrentRepository(GitInterface *repository)
{
  _impl->currentRepository = _impl->repositories.indexOf(repository);
  _impl->changeRepository();
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

  if (found == _impl->repositories.end())
  {
    return;
  }

  _impl->currentRepository = _impl->repositories.indexOf(*found);
  _impl->changeRepository();
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

void Project::reloadAllRepositories()
{
  for (auto &repository : repositoryList())
  {
    repository->reload();
  }
}
