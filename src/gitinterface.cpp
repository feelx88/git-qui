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
    _impl(new GitInterfacePrivate)
{
  _impl->repositoryPath = repositoryPath;
  _impl->process = new QProcess(this);
  _impl->process->setWorkingDirectory(repositoryPath.absolutePath());
  _impl->process->setProgram("git");
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
  _impl->process->setArguments({
    "status",
    "--untracked=all",
    "--porcelain=v1",
    "-z",
  });

  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  QList<GitFile> unstaged, staged;

  for(auto output : _impl->process->readAll().split('\0'))
  {
    if(output.isEmpty())
    {
      continue;
    }

    GitFile file;

    char firstByte = output.at(0);
    char secondByte = output.at(1);

    file.staged = (firstByte != ' ' && firstByte != '?');
    file.unstaged = (secondByte != ' ');
    file.path = output.right(output.length() - 3).trimmed();

    switch(firstByte)
    {
    case 'M':
      file.modified = true;
      break;
    case 'D':
      file.deleted = true;
      break;
    case 'A':
      file.added = true;
      break;
    case 'R':
    case 'C':
    default:
      break;
    }

    switch(secondByte)
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

    if (file.staged)
    {
      staged.append(file);
    }
  }

  emit nonStagingAreaChanged(unstaged);
  emit stagingAreaChanged(staged);
}

void GitInterface::log()
{
  _impl->process->setArguments({"log",
                                  "--all",
                                  "--graph",
                                  "--full-history",
                                  "--pretty="
                                  "%x0c"
                                  "%H"
                                  "%x0c"
                                  "%s"
                                 });
    _impl->process->start(QIODevice::ReadOnly);
    _impl->process->waitForFinished();

    QVariantList list;
    QHash<QString, GitCommit*> map;

    while(true)
    {
      if(_impl->process->atEnd())
      {
        break;
      }

      QString buf;
      QByteArray c = _impl->process->read(1);
      while(c != "\n")
      {
        buf += c;
        c = _impl->process->read(1);
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
  _impl->process->setArguments({"commit",
                                  "--message",
                                  message,
                                 });
    _impl->process->start(QIODevice::ReadOnly);
    _impl->process->waitForFinished();

  reload();
}
