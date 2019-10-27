#include "project.hpp"

#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <QFileDialog>

struct ProjectImpl
{
  static inline const QString CONFIG_NAME = "name";
  static inline const QString CONFIG_REPOSITORY_LIST = "repositoryList";

  static inline const QString CONFIG_REPOSITORY_LIST_NAME = "name";
  static inline const QString CONFIG_REPOSITORY_LIST_PATH = "path";

  QSettings *settings = nullptr;
  QString name;
  QList<Repository*> repositories;
  QList<QRegularExpression> ignoredSubdirectories = {
    QRegularExpression("/.*vendor.*/"),
    QRegularExpression("/.*node_modules.*/")
  };

  void loadSettings(Project *_this)
  {
    name = settings->value(CONFIG_NAME).toString();

    QList<QVariantMap> list = qvariant_cast<QList<QVariantMap>>(settings->value(CONFIG_REPOSITORY_LIST));

    for (auto entry : list)
    {
      repositories.append(new Repository(
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
      QList<QVariantMap> list;

      for (auto repository : repositories)
      {
        list << QVariantMap {
          {CONFIG_REPOSITORY_LIST_NAME, repository->name},
          {CONFIG_REPOSITORY_LIST_PATH, repository->path.path()}
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
  _impl(new ProjectImpl)
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

QList<Repository*> Project::repositoryList() const
{
  return _impl->repositories;
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
        _impl->repositories.append(new Repository(currentDir.dirName(), currentDir.absolutePath(), this));
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

void Project::updateRepository(const int &index, Repository *repository)
{
  _impl->repositories.replace(index, repository);
  _impl->writeSettings();
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
    _impl->loadSettings(this);
    _impl->writeSettings();
  }
}
