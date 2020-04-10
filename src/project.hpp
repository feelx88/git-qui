#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QObject>
#include <QList>

struct ProjectImpl;
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

  GitInterface *activeRepository();

  void addRepository();
  void removeRepository(const int &index);

  void setCurrentRepository(const int &index);
  void setCurrentRepository(GitInterface *repository);
  void setCurrentRepository(const QString &name);

  void setName(const QString &name);
  void setFileName(const QString &fileName);

  void save();

signals:
  void repositoryAdded(GitInterface *repository);
  void repositorySwitched(GitInterface *repository);
  void repositoryRemoved(GitInterface *repository);

private:
  ProjectImpl *_impl;
};

#endif // PROJECT_HPP
