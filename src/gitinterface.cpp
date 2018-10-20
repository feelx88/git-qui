#include "gitinterface.hpp"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDebug>

#include "gitcommit.hpp"

class GitInterfacePrivate
{
public:
  QDir repositoryPath;
  QProcess *process;
};

GitInterface::GitInterface(QObject *parent, const QDir &repositoryPath)
  : QObject(parent),
    _data(new GitInterfacePrivate)
{
  _data->repositoryPath = repositoryPath;
  _data->process = new QProcess(this);
  _data->process->setWorkingDirectory(repositoryPath.absolutePath());
  _data->process->setProgram("git");
}

GitInterface::~GitInterface()
{
}

void GitInterface::reload()
{

}

void GitInterface::log()
{
  _data->process->setArguments({"log",
                                  "--all",
                                  "--graph",
                                  "--full-history",
                                  "--pretty="
                                  "%x0c"
                                  "%H"
                                  "%x0c"
                                  "%s"
                                 });
    _data->process->start(QIODevice::ReadOnly);
    _data->process->waitForFinished();

    QVariantList list;
    QHash<QString, GitCommit*> map;

    while(true)
    {
      if(_data->process->atEnd())
      {
        break;
      }

      QString buf;
      QByteArray c = _data->process->read(1);
      while(c != "\n")
      {
        buf += c;
        c = _data->process->read(1);
      }
      QStringList parts = buf.split('\f');

      GitCommit *commit = new GitCommit(this);
      if (parts.length() > 1)
      {
        commit->id = parts.at(1);
        commit->message = parts.at(2);
      }

      list.append(QVariant::fromValue(commit));
    }

    emit logChanged(list);
}

void GitInterface::commit(const QString &message)
{
  _data->process->setArguments({"commit",
                                  "--message",
                                  message,
                                 });
    _data->process->start(QIODevice::ReadOnly);
    _data->process->waitForFinished();

  log();
  emit stagingAreaChanged();
  emit nonStagingAreaChanged();
}
