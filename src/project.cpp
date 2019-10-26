#include "project.hpp"

#include <QSettings>
#include <qfiledialog.h>

struct ProjectImpl
{
  QSettings *settings = nullptr;
  QString name;

  void updateSettings()
  {
    if (settings)
    {
      settings->setValue("name", name);
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

void Project::repositoryList(const QList<Repository> &repositoryList)
{
  _impl->settings->setValue("repositoryList", QVariant::fromValue(repositoryList));
}

QString Project::name() const
{
  return _impl->settings ? _impl->settings->value("name").toString() : _impl->name;
}

QList<Repository> Project::repositoryList() const
{
  return _impl->settings ? qvariant_cast<QList<Repository>>(_impl->settings->value("repositoryList")) : QList<Repository>();
}

void Project::addRepository()
{
  QString path = QFileDialog::getExistingDirectory(
    nullptr,
    "Select repository path",
    QDir::current().path()
  );

  Repository repository;
  repository.path.setPath(path);
  repository.name = repository.path.dirName();

  if (!path.isNull())
  {
    _impl->settings->setValue("repositoryList", QVariant::fromValue(repositoryList() << repository));
    _impl->settings->sync();
  }
}

void Project::removeRepository(const int &index)
{
  auto list = repositoryList();
  list.removeAt(index);
  repositoryList(list);
}

void Project::updateRepository(const int &index, const Repository &repository)
{
  auto list = repositoryList();
  list.replace(index, repository);
  repositoryList(list);
}

void Project::setName(const QString &name)
{
  _impl->name = name;
  _impl->updateSettings();
}

void Project::setFileName(const QString &fileName)
{
  if (!_impl->settings)
  {
    _impl->settings = new QSettings(fileName, QSettings::IniFormat, this);
    _impl->updateSettings();

    _impl->name = name();
  }
}
