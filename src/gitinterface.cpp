#include "include/gitinterface.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDebug>

#include "gitcommit.h"

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
                                  "--reverse",
                                  "--pretty="
                                  "%H%x0c"
                                  "%P%x0c"
                                  "%D%x0c"
                                  "%cn%x0c"
                                  "%ce%x0c"
                                  "%ct%x0c"
                                  "%s%x0c"
                                  "%b%x0b",
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
      while(c != "\v")
      {
        buf += c;
        c = _data->process->read(1);
      }
      _data->process->read(1);
      QStringList parts = buf.split('\f');

      GitCommit *commit = new GitCommit(this);
      commit->id = parts.at(0);
      commit->branches = parts.at(2).split(", ", QString::SkipEmptyParts);
      commit->message = parts.at(6);
      list.append(QVariant::fromValue(commit));
      map.insert(commit->id, commit);

      QStringList parents = parts.at(1).split(" ", QString::SkipEmptyParts);

      for (QString parent : parents)
      {
        GitCommit *parentCommit = map.value(parent);
        if(parentCommit)
        {
          commit->parents.append(QVariant::fromValue(parentCommit));
        }
      }
    }

    _data->process->setArguments({"rev-list",
                                    "--all",
                                    "--children",
                                   });
    _data->process->start(QIODevice::ReadOnly);
    _data->process->waitForFinished();

    while(true)
    {
      if(_data->process->atEnd())
      {
        break;
      }

      QString line = _data->process->readLine();
      QStringList children = line.split(" ");

      GitCommit *commit = map.value(children.at(0));

      if (commit) {
        children.pop_front();

        for (QString child : children) {
          child = child.trimmed();
          commit->children.append(QVariant::fromValue(map.value(child)));
        }
      }
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
