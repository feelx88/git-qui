#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QList>

#include "repository.hpp"

struct ProjectImpl;

class Project : public QObject
{
  Q_OBJECT
public:
  Project(const QString &fileName, QObject *parent = nullptr);

  QString name() const;
  QList<Repository> repositoryList() const;
  void repositoryList(const QList<Repository> &repositoryList);

  void addRepository(const Repository &repository);

private:
  ProjectImpl *_impl;
};

#endif // PROJECT_HPP
