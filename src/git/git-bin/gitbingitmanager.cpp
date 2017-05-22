#include <git/git-bin/gitbingitmanager.h>

#include <QProcess>

#include <git/gitfile.h>
#include <git/gitdiffline.h>

struct gitBin::GitManager::GitManagerPrivate
{
  QProcess *process;
};

gitBin::GitManager::GitManager(QObject *parent)
  : AGitManager(parent),
    _impl(new GitManagerPrivate)
{
  _impl->process = new QProcess(this);
  _impl->process->setProgram("git");
}

void gitBin::GitManager::init()
{
}

void gitBin::GitManager::openRepository(const QString &)
{
}

QList<GitFile *> gitBin::GitManager::status()
{
  _impl->process->setArguments({"status", "--porcelain=v1"});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
  QByteArray output = _impl->process->readLine(1024);

  QList<GitFile*> list;

  while (output.length() > 0)
  {
    GitFile *file = new GitFile(this);

    file->staged = (output.at(0) != ' ');
    file->unstaged = (output.at(1) != ' ');
    file->path = output.right(output.length() - 3).trimmed();

    switch(output.at(0))
    {
    case 'M':
      file->modified = true;
      break;
    case 'D':
      file->deleted = true;
      break;
    case 'R':
    case 'C':
    default:
      break;
    }

    switch(output.at(1))
    {
    case 'M':
      file->modified = true;
      break;
    case 'D':
      file->deleted = true;
      break;
    case 'R':
    case 'C':
    default:
      break;
    }

    list.append(file);

    output = _impl->process->readLine(1024);
  }

  return list;
}

QList<GitDiffLine *> gitBin::GitManager::diffPath(const QString &path, bool diffStaged)
{
  if(diffStaged)
  {
    _impl->process->setArguments({"diff", "--cached", "--", path});
  }
  else
  {
    _impl->process->setArguments({"diff", "--", path});
  }
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
  QByteArray output = _impl->process->readLine(1024);
  QRegExp regex("@* \\-(\\d+),.* \\+(\\d+),.*");
  QStringList lineNos;

  QList<GitDiffLine*> list;

  int lineNoOld = -1;
  int lineNoNew = -1;

  while(output.length() > 0)
  {
    GitDiffLine *line = new GitDiffLine(this);
    line->oldLine = lineNoOld;
    line->newLine = lineNoNew;
    line->content = output.trimmed();
    line->type = GitDiffLine::diffType::FILE_HEADER;


    if(output.startsWith("index")
       || output.startsWith("diff")
       || output.startsWith("+++")
       || output.startsWith("---"))
    {
      line->type = GitDiffLine::diffType::HEADER;
    }
    else
    {
      switch(output.at(0))
      {
      case '@':
        line->type = GitDiffLine::diffType::HEADER;
        regex.indexIn(line->content);
        lineNos = regex.capturedTexts();
        lineNoOld = lineNos.at(1).toInt();
        lineNoNew = lineNos.at(2).toInt();
        break;
      case '+':
        line->content = line->content.right(line->content.length() - 1);
        line->type = GitDiffLine::diffType::ADD;
        line->oldLine = -1;
        lineNoNew++;
        break;
      case '-':
        line->content = line->content.right(line->content.length() - 1);
        line->type = GitDiffLine::diffType::REMOVE;
        line->newLine = -1;
        lineNoOld++;
        break;
      default:
        line->type = GitDiffLine::diffType::CONTEXT;
        lineNoOld++;
        lineNoNew++;
        break;
      }
    }

    list.append(line);
    output = _impl->process->readLine(1024);
  }

  return list;
}

QString gitBin::GitManager::headName()
{
  _impl->process->setArguments({"branch"});
  _impl->process->start(QIODevice::ReadOnly);

  return QString(_impl->process->readAll());
}

void gitBin::GitManager::stagePath(const QString &path)
{
  _impl->process->setArguments({"add", path});
  _impl->process->start();
  _impl->process->waitForFinished();
}

void gitBin::GitManager::unstagePath(const QString &path)
{
  _impl->process->setArguments({"reset", "HEAD", path});
  _impl->process->start();
  _impl->process->waitForFinished();
}

void gitBin::GitManager::commit(const QString &message)
{
  _impl->process->setArguments({"commit", "-m", message});
  _impl->process->start();
  _impl->process->waitForFinished();
}
