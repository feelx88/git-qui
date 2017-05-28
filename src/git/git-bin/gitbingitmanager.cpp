#include <git/git-bin/gitbingitmanager.h>

#include <QProcess>
#include <QtDebug>

#include <git/gitfile.h>
#include <git/gitdiffline.h>
#include <git/gitcommit.h>

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

void gitBin::GitManager::openRepository(const QString &path)
{
  QString root = repositoryRoot(path);

  if (root.length() > 0)
  {
    _impl->process->setWorkingDirectory(root);
  }
  else
  {
    emit gitError(QString("Not a git repository: ") + path);
  }
}

QString gitBin::GitManager::repositoryRoot(const QString &)
{
  _impl->process->setArguments({"rev-parse",  "--show-toplevel"});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  if(_impl->process->exitCode() == QProcess::NormalExit)
  {
    QString output = QString(_impl->process->readLine(1024)).trimmed();
    return output;
  }
  else
  {
    return "";
  }
}

QList<GitFile *> gitBin::GitManager::status()
{
  _impl->process->setArguments({"status", "-uall", "--porcelain=v1"});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
  QByteArray output = _impl->process->readLine(1024);

  QList<GitFile*> list;

  while (output.length() > 0)
  {
    GitFile *file = new GitFile(this);

    file->staged = (output.at(0) != ' ' && output.at(0) != '?');
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
  QList<GitDiffLine*> list;

  if(path.isEmpty())
  {
    return list;
  }

  if(diffStaged)
  {
    _impl->process->setArguments({"diff", "HEAD", "--cached", "--", path});
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
  int index = 0;

  int lineNoOld = -1;
  int lineNoNew = -1;

  if(output.length() == 0)
  {
    _impl->process->setProgram("sed");
    _impl->process->setArguments({"s/^/+ /", path});

    _impl->process->start(QIODevice::ReadOnly);
    _impl->process->waitForFinished();
    output = _impl->process->readLine(1024);

    lineNoOld = -1;
    lineNoNew = 1;

    GitDiffLine *header = new GitDiffLine(this);
    header->type = GitDiffLine::diffType::HEADER;
    header->content = "untracked file";

    list.append(header);

    _impl->process->setProgram("git");
  }

  QString header;

  while(output.length() > 0)
  {
    GitDiffLine *line = new GitDiffLine(this);
    line->oldLine = lineNoOld;
    line->newLine = lineNoNew;
    line->index = index++;

    while(output.length() > 2 && (output.endsWith(' ') || output.endsWith('\n') || output.endsWith('\t')))
    {
      output.chop(1);
    }
    line->content = output;

    line->type = GitDiffLine::diffType::FILE_HEADER;

    if(output.startsWith("index")
       || output.startsWith("diff")
       || output.startsWith("+++")
       || output.startsWith("---")
       || output.startsWith("new file")
       || output.startsWith("old file"))
    {
      line->type = GitDiffLine::diffType::HEADER;
      header += output + '\n';
    }
    else
    {
      line->header = header;
      switch(output.at(0))
      {
      case '@':
        line->type = GitDiffLine::diffType::HEADER;
        line->oldLine = -1;
        line->newLine = -1;
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
        line->content = line->content.right(line->content.length() - 1);
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

void gitBin::GitManager::stageLines(const QList<GitDiffLine *> &lines, bool reverse)
{
  if (lines.isEmpty())
  {
    return;
  }

  GitDiffLine *first = lines.first();
  QString patch = first->header;
  QList<QList<GitDiffLine*>> hunks;
  hunks.append(QList<GitDiffLine*>());
  int lastindex = (first->type == GitDiffLine::diffType::ADD) ? first->newLine : first->oldLine;

  for(auto line : lines)
  {
    if(!(line->index - lastindex <= 1))
    {
      hunks.append(QList<GitDiffLine*>());
    }

    hunks.last().append(line);
    lastindex = line->index;
  }

  for(auto hunk : hunks)
  {
    int newCount = 0, oldCount = 0;
    GitDiffLine *first = hunk.first();

    for (GitDiffLine* line : hunk)
    {
      if(line->type == GitDiffLine::diffType::ADD)
      {
        ++newCount;
      }
      else if(line->type == GitDiffLine::diffType::REMOVE
              || line->type == GitDiffLine::diffType::CONTEXT)
      {
        ++oldCount;
      }
    }

    if(first->oldLine < 0)
    {
      first->oldLine = first->newLine;
    }

    patch.append(QString::asprintf("@@ -%i,%i +%i,%i @@\n", first->oldLine, oldCount, first->oldLine, newCount));

    for (GitDiffLine* line : hunk)
    {
      if(line->type == GitDiffLine::diffType::ADD)
      {
        patch += "+";
      }
      else if(line->type == GitDiffLine::diffType::REMOVE)
      {
        patch += "-";
      }
      patch += line->content + '\n';
    }
  }

  if(reverse)
  {
    _impl->process->setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "--reverse", "-"});
  }
  else
  {
    _impl->process->setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "-"});
  }

  qDebug().noquote() << patch;

  std::string stdPatch = patch.toStdString();
  _impl->process->start(QIODevice::ReadWrite);
  _impl->process->waitForStarted();
  _impl->process->write(stdPatch.data(), stdPatch.length());
  _impl->process->closeWriteChannel();
  _impl->process->waitForFinished();
}

QList<GitCommit *> gitBin::GitManager::log()
{
  _impl->process->setArguments({"log", "--oneline"});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  QList<GitCommit*> list;

  for(QString output = _impl->process->readLine(1024); !output.isEmpty(); output = _impl->process->readLine(1024))
  {
    GitCommit *commit = new GitCommit(this);
    commit->id = output;
    list.append(commit);
  }

  return list;
}

QString gitBin::GitManager::headName()
{
  _impl->process->setArguments({"rev-parse", "--abbrev-ref", "HEAD"});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  return QString(_impl->process->readAll().trimmed());
}

void gitBin::GitManager::stagePath(const QString &path)
{
  _impl->process->setArguments({"add", path});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
}

void gitBin::GitManager::unstagePath(const QString &path)
{
  _impl->process->setArguments({"reset", "HEAD", path});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
}

void gitBin::GitManager::commit(const QString &message)
{
  _impl->process->setArguments({"commit", "-m", message});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
}

void gitBin::GitManager::checkout(const QString &path)
{
  _impl->process->setArguments({"checkout", path});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();
}

void gitBin::GitManager::push(const QString &branch, const QString &remote, const QString &remoteBranch)
{
  _impl->process->setArguments({"push", remote, branch + ":" + remoteBranch});
  _impl->process->start(QIODevice::ReadOnly);

  auto con = std::make_shared<QMetaObject::Connection>();
  *con = connect(_impl->process, QOverload<int>::of(&QProcess::finished), [=] {
    emit gitError(_impl->process->readAllStandardError().trimmed());
    disconnect(*con);
  });
}
