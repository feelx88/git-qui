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
  Project(QObject *parent = nullptr);

  QString fileName() const;
  QString name() const;
  QList<Repository *> repositoryList() const;

  void addRepository();
  void removeRepository(const int &index);
  void updateRepository(const int &index, Repository *repository);

  void setName(const QString &name);
  void setFileName(const QString &fileName);

private:
  ProjectImpl *_impl;
};

#endif // PROJECT_HPP
