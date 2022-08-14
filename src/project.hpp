#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QList>
#include <QObject>
#include <QSharedPointer>

struct ProjectPrivate;
class GitInterface;

class Project : public QObject {
  Q_OBJECT
public:
  Project(const QString &fileName, QObject *parent = nullptr);
  Project(QObject *parent = nullptr);

  QString fileName() const;
  QString name() const;
  QList<QSharedPointer<GitInterface>> repositoryList() const;

  QSharedPointer<GitInterface> activeRepository() const;
  QSharedPointer<QObject> activeRepositoryContext() const;
  QSharedPointer<GitInterface> repositoryByName(const QString &name) const;

  QVariantMap dockWidgetConfiguration() const;
  void setDockWidgetConfigurationEntry(const QString &key, QVariant value);

  void addRepository();
  void removeRepository(const int &index);

  void setCurrentRepository(const int &index);
  void setCurrentRepository(const QString &name);

  void setName(const QString &name);
  void setFileName(const QString &fileName);

  void save();

  void reloadAllRepositories();

signals:
  void repositoryAdded(QSharedPointer<GitInterface> repository);
  void repositorySwitched(QSharedPointer<GitInterface> repository,
                          QSharedPointer<QObject> activeRepositoryContext);
  void repositoryRemoved(QSharedPointer<GitInterface> repository);

private:
  QSharedPointer<ProjectPrivate> _impl;
};

#endif // PROJECT_HPP
