#include "project.hpp"

#include <QSettings>

struct ProjectImpl
{
  QSettings *settings;
};

Project::Project(const QString &fileName, QObject *parent)
  : QObject(parent),
    _impl(new ProjectImpl)
{
  _impl->settings = new QSettings(fileName, QSettings::IniFormat, this);
}

void Project::repositoryList(const QList<Repository> &repositoryList)
{
  _impl->settings->setValue("repositoryList", QVariant::fromValue(repositoryList));
}

QString Project::name() const
{
  return _impl->settings->value("name").toString();
}

QList<Repository> Project::repositoryList() const
{
  return qvariant_cast<QList<Repository>>(_impl->settings->value("repositoryList"));
}

void Project::addRepository(const Repository &repository)
{
  _impl->settings->setValue("repositoryList", QVariant::fromValue(repositoryList() << repository));
}
