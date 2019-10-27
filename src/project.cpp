#include "project.hpp"

#include <QDirIterator>
#include <QRegularExpression>
#include <QSettings>
#include <qfiledialog.h>

struct ProjectImpl
{
  QSettings *settings = nullptr;
  QString name;
  QList<QRegularExpression> ignoredSubdirectories = {
    QRegularExpression("/.*vendor.*/"),
    QRegularExpression("/.*node_modules.*/")
  };

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
        list << Repository(currentDir.dirName(), currentDir.absolutePath(), this);
      }
    }

    _impl->settings->setValue("repositoryList", QVariant::fromValue(list));
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
