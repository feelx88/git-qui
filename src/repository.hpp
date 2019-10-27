#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <QObject>
#include <QString>
#include <QDir>
#include <QDataStream>

class GitInterface;

struct RepositoryImpl;

class Repository : public QObject
{
  Q_OBJECT
public:
  explicit Repository(const QString &name = "", const QString &path = "", QObject *parent = nullptr);
  explicit Repository(const Repository &other);
  Repository &operator=(const Repository &other);

  QString name;
  QDir path;
  GitInterface *gitInterface;

private:
  RepositoryImpl *_impl;
};

Q_DECLARE_METATYPE(Repository)

#endif // REPOSITORY_HPP
