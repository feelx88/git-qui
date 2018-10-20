#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <memory>
#include <QObject>
#include <QVariant>

#include "gitfile.hpp"

class QDir;
class QFile;
class GitCommit;

class GitInterfacePrivate;

class GitInterface : public QObject
{
  Q_OBJECT
public:
  GitInterface(QObject *parent, const QDir &repositoryPath);
  virtual ~GitInterface();
public slots:
  void reload();
  void status();
  void log();
  void commit(const QString& message);
signals:
  void fileChanged(const QFile& fileName);
  void nonStagingAreaChanged(QList<GitFile>);
  void stagingAreaChanged(QList<GitFile>);
  void logChanged(QVariantList logs);

private:
  std::unique_ptr<GitInterfacePrivate> _data;
};

#endif // GITINTERFACE_H
