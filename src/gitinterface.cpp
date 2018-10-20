#include "gitinterface.hpp"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDebug>

#include "gitcommit.hpp"
#include "gitfile.hpp"

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
  status();
  log();
}

void GitInterface::status()
{
  _data->process->setArguments({
    "status",
    "--untracked=all",
    "--porcelain=v1",
    "-z",
  });

  _data->process->start(QIODevice::ReadOnly);
  _data->process->waitForFinished();

  QList<GitFile> unstaged, staged;

  for(auto output : _data->process->readAll().split('\0'))
  {
    if(output.isEmpty())
    {
      continue;
    }

    GitFile file;

    file.staged = (output.at(0) != ' ' && output.at(0) != '?');
    file.unstaged = (output.at(1) != ' ');
    file.path = output.right(output.length() - 3).trimmed();

    switch(output.at(0))
    {
    case 'M':
      file.modified = true;
      break;
    case 'D':
      file.deleted = true;
      break;
    case 'R':
    case 'C':
    default:
      break;
    }

    switch(output.at(1))
    {
    case 'M':
      file.modified = true;
      break;
    case 'D':
      file.deleted = true;
      break;
    case 'R':
    case 'C':
    default:
      break;
    }

    if (file.unstaged)
    {
      unstaged.append(file);
    }
    else
    {
      staged.append(file);
    }
  }

  emit nonStagingAreaChanged(unstaged);
  emit stagingAreaChanged(staged);
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

  reload();
}
