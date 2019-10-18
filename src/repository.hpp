#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <QString>
#include <QDir>
#include <QDataStream>

class Repository
{
public:
  Repository();

  Q_PROPERTY(QDir name MEMBER name)
  QString name;

  Q_PROPERTY(QDir path MEMBER path)
  QDir path;
};

Q_DECLARE_METATYPE(Repository)
QDataStream &operator<<(QDataStream &out, const Repository &repository);
QDataStream &operator>>(QDataStream &in, Repository &repository);

#endif // REPOSITORY_HPP
