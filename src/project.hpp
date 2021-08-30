#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QObject>
#include <QList>

struct ProjectPrivate;
class GitInterface;

class Project : public QObject
{
  Q_OBJECT
public:
  Project(const QString &fileName, QObject *parent = nullptr);
  Project(QObject *parent = nullptr);

  QString fileName() const;
  QString name() const;
  QList<GitInterface *> repositoryList() const;

  GitInterface *activeRepository() const;
  QObject *activeRepositoryContext() const;
  GitInterface *repositoryByName(const QString &name) const;

  QVariantMap dockWidgetConfiguration() const;
  void setDockWidgetConfigurationEntry(const QString& key, QVariant value);

  void addRepository();
  void removeRepository(const int &index);

  void setCurrentRepository(const int &index);
  void setCurrentRepository(GitInterface *repository);
  void setCurrentRepository(const QString &name);

  void setName(const QString &name);
  void setFileName(const QString &fileName);

  void save();

  void reloadAllRepositories();

signals:
  void repositoryAdded(GitInterface *repository);
  void repositorySwitched(GitInterface *repository, QObject *activeRepositoryContext);
  void repositoryRemoved(GitInterface *repository);

private:
  ProjectPrivate *_impl;
};

#endif // PROJECT_HPP
