#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <QString>
#include <QDir>

class Repository
{
public:
  Repository();

  Q_PROPERTY(QDir name MEMBER name)
  Q_PROPERTY(QDir path MEMBER path)

private:
  QString name;
  QDir path;
};

#endif // REPOSITORY_HPP
