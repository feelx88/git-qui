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
  QProcess *foregroundProcess, *backgroundProcess;
  bool readyForCommit = false;
  bool fullFileDiff = false;

  void connect()
  {
    QObject::connect(foregroundProcess, static_cast<void(QProcess::*)(int)>(&QProcess::finished), foregroundProcess, [=](int){
      auto output = foregroundProcess->readAllStandardError();
      if (!output.isEmpty())
      {
        qDebug() << output;
      }
    });
  }

  void callAsyncSingle(QProcess *process, std::function<void(int)> lambda)
  {
    QObject *context = new QObject;
    process->connect(process, static_cast<void(QProcess::*)(int)>(&QProcess::finished), context, [=](int exitCode){
      lambda(exitCode);
      delete context;
    });
  }
};

GitInterface::GitInterface(QObject *parent, const QString &path)
  : QObject(parent),
    _impl(new GitInterfacePrivate)
{
  _impl->foregroundProcess = new QProcess(this);
  _impl->foregroundProcess->setProgram("git");
  _impl->backgroundProcess = new QProcess(this);
  _impl->backgroundProcess->setProgram("git");

  _impl->connect();

  switchRepository(path);
}

GitInterface::~GitInterface()
{
}

void GitInterface::switchRepository(const QString &path)
{
  _impl->repositoryPath = path;
  _impl->foregroundProcess->setWorkingDirectory(path);
  _impl->backgroundProcess->setWorkingDirectory(path);
  emit repositorySwitched(path);
  reload();
}

void GitInterface::reload()
{
  if (_impl->foregroundProcess->state() == QProcess::Running)
  {
    return;
  }
  status();
  log();
  emit reloaded();
}

void GitInterface::status()
{
  _impl->foregroundProcess->setArguments({
    "status",
    "--untracked=all",
    "--porcelain=v1",
    "-z",
  });

  _impl->foregroundProcess->start(QIODevice::ReadOnly);
  _impl->foregroundProcess->waitForFinished();

  QList<GitFile> unstaged, staged;

  for(auto output : _impl->foregroundProcess->readAll().split('\0'))
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

  _impl->readyForCommit = !staged.empty();

  emit nonStagingAreaChanged(unstaged);
  emit stagingAreaChanged(staged);

  _impl->foregroundProcess->setArguments({
    "rev-parse",
    "--abbrev-ref",
    "HEAD"
  });

  _impl->foregroundProcess->start(QIODevice::ReadOnly);
  _impl->foregroundProcess->waitForFinished();

  emit branchChanged(_impl->foregroundProcess->readAll().trimmed(), !(unstaged.empty() && staged.empty()));
}

void GitInterface::log()
{
  _impl->foregroundProcess->setArguments({"log",
                                  "--all",
                                  "--graph",
                                  "--full-history",
                                  "--pretty="
                                  "%x0c"
                                  "%H"
                                  "%x0c"
                                  "%s"
                                 });
    _impl->foregroundProcess->start(QIODevice::ReadOnly);
    _impl->foregroundProcess->waitForFinished();

    QVariantList list;
    QHash<QString, GitCommit*> map;

    while(true)
    {
      if(_impl->foregroundProcess->atEnd())
      {
        break;
      }

      QString buf;
      QByteArray c = _impl->foregroundProcess->read(1);
      while(c != "\n")
      {
        buf += c;
        c = _impl->foregroundProcess->read(1);
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
  if (!_impl->readyForCommit)
  {
    emit error(tr("There are no files to commit"));
    return;
  }

  _impl->foregroundProcess->setArguments({"commit",
                                  "--message",
                                  message,
                                 });
  _impl->foregroundProcess->start(QIODevice::ReadOnly);
  _impl->foregroundProcess->waitForFinished();

  reload();
  emit commited();
}

void GitInterface::stageFile(const QString &path)
{
  _impl->foregroundProcess->setArguments({"add", path});
  _impl->foregroundProcess->start(QIODevice::ReadOnly);
  _impl->foregroundProcess->waitForFinished();

  status();
}

void GitInterface::unstageFile(const QString &path)
{
  _impl->foregroundProcess->setArguments({"reset", "HEAD", path});
  _impl->foregroundProcess->start(QIODevice::ReadOnly);
  _impl->foregroundProcess->waitForFinished();

  status();
}

void GitInterface::selectFile(bool unstaged, const QString &path)
{
  emit fileSelected(unstaged, path);

  diffFile(unstaged, path);
}

void GitInterface::diffFile(bool unstaged, const QString &path)
{
  QString absolutePath = _impl->repositoryPath.absolutePath() + "/" + path;

  if (path.isEmpty() || !QFileInfo(absolutePath).isFile())
  {
    return;
  }

  bool binary = false;
  int lineCount = 0;

  QFile *file = new QFile(absolutePath, this);
  file->open(QIODevice::ReadOnly);

  QByteArray tmp = file->read(1024 * 512); //512kiB
  if(tmp.size() > 0 && tmp.contains('\0'))
  {
    binary = true;
  }
  else
  {
    file->reset();
    while(!file->readLine(0).isEmpty())
    {
      ++lineCount;
    }
  }

  QList<GitDiffLine> list;

  if(unstaged)
  {
    _impl->foregroundProcess->setArguments({
      "diff",
      _impl->fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
      "--",
      path
    });
  }
  else
  {
    _impl->foregroundProcess->setArguments({
      "diff",
      _impl->fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
      "HEAD",
      "--cached",
      "--",
      path
    });
  }
  _impl->foregroundProcess->start(QIODevice::ReadOnly);
  _impl->foregroundProcess->waitForFinished();

  QByteArray output = _impl->foregroundProcess->readLine();
  QRegExp regex("@* \\-(\\d+),.* \\+(\\d+),.*");
  QStringList lineNos;
  int index = 0;

  int lineNoOld = -1;
  int lineNoNew = -1;

  auto readLine = new std::function<QByteArray()>([&] {
    return _impl->foregroundProcess->readLine();
  });

  if(output.length() == 0)
  {
    lineNoOld = -1;
    lineNoNew = 1;

    GitDiffLine header;
    header.type = GitDiffLine::diffType::HEADER;
    header.content = "untracked file";

    list.append(header);

    file->reset();

    readLine = new std::function<QByteArray()>([&] {
      QByteArray arr(file->readLine());
      if (arr.size() > 0)
      {
        arr.prepend("+ ");
      }
      return arr;
    });

    output = (*readLine)();
  }

  QString header;

  if(binary)
  {
    GitDiffLine line;
    line.type = GitDiffLine::diffType::HEADER;
    line.content = "binary file";
    list.append(line);
  }
  else
  {
    while(output.length() > 0)
    {
      GitDiffLine line;
      line.oldLine = lineNoOld;
      line.newLine = lineNoNew;
      line.index = index++;

      while(output.length() > 2 && (output.endsWith(' ') || output.endsWith('\n') || output.endsWith('\t')))
      {
        output.chop(1);
      }
      line.content = output;

      line.type = GitDiffLine::diffType::FILE_HEADER;

      if(output.startsWith("index")
         || output.startsWith("diff")
         || output.startsWith("+++")
         || output.startsWith("---")
         || output.startsWith("new file")
         || output.startsWith("old file"))
      {
        line.type = GitDiffLine::diffType::HEADER;
        header += output + '\n';
      }
      else
      {
        line.header = header;
        switch(output.at(0))
        {
        case '@':
          line.type = GitDiffLine::diffType::HEADER;
          line.oldLine = -1;
          line.newLine = -1;
          regex.indexIn(line.content);
          lineNos = regex.capturedTexts();
          lineNoOld = lineNos.at(1).toInt();
          lineNoNew = lineNos.at(2).toInt();
          break;
        case '+':
          line.content = line.content.right(line.content.length() - 1);
          line.type = GitDiffLine::diffType::ADD;
          line.oldLine = -1;
          lineNoNew++;
          break;
        case '-':
          line.content = line.content.right(line.content.length() - 1);
          line.type = GitDiffLine::diffType::REMOVE;
          line.newLine = -1;
          lineNoOld++;
          break;
        default:
          line.content = line.content.right(line.content.length() - 1);
          line.type = GitDiffLine::diffType::CONTEXT;
          lineNoOld++;
          lineNoNew++;
          break;
        }
      }

      list.append(line);
      output = (*readLine)();
    }
  }

  delete readLine;
  emit fileDiffed(path, list, unstaged);
}

void GitInterface::addLines(const QList<GitDiffLine> &lines, bool unstage)
{
  if (lines.isEmpty())
  {
    return;
  }

  GitDiffLine first = lines.first();
  QString patch = first.header;
  QList<QList<GitDiffLine>> hunks;
  hunks.append(QList<GitDiffLine>());
  int lastindex = first.index;

  for(auto line : lines)
  {
    if(!(line.index - lastindex <= 1))
    {
      hunks.append(QList<GitDiffLine>());
    }

    hunks.last().append(line);
    lastindex = line.index;
  }

  for(auto hunk : hunks)
  {
    int newCount = 0, oldCount = 0;
    GitDiffLine first = hunk.first();

    for (GitDiffLine line : hunk)
    {
      if(line.type == GitDiffLine::diffType::ADD)
      {
        ++newCount;
      }
      else if(line.type == GitDiffLine::diffType::REMOVE
              || line.type == GitDiffLine::diffType::CONTEXT)
      {
        ++oldCount;
      }
    }

    if(first.oldLine < 0)
    {
      first.oldLine = first.newLine;
    }

    patch.append(QString::asprintf("@@ -%i,%i +%i,%i @@\n", first.oldLine, oldCount, first.oldLine, newCount));

    for (GitDiffLine line : hunk)
    {
      if(line.type == GitDiffLine::diffType::ADD)
      {
        patch += "+";
      }
      else if(line.type == GitDiffLine::diffType::REMOVE)
      {
        patch += "-";
      }
      patch += line.content.remove('\n') + '\n';
    }
  }

  if(unstage)
  {
    _impl->foregroundProcess->setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "-"});
  }
  else
  {
    _impl->foregroundProcess->setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "--reverse", "-"});
  }

  qDebug().noquote() << patch;

  std::string stdPatch = patch.toStdString();
  _impl->foregroundProcess->start(QIODevice::ReadWrite);
  _impl->foregroundProcess->waitForStarted();
  _impl->foregroundProcess->write(stdPatch.data(), stdPatch.length());
  _impl->foregroundProcess->closeWriteChannel();
  _impl->foregroundProcess->waitForFinished();

  status();
}

void GitInterface::push()
{
  _impl->backgroundProcess->setArguments({
    "push"
  });
  _impl->backgroundProcess->start();

  _impl->callAsyncSingle(_impl->backgroundProcess, [=](int exitCode){
    if (exitCode != 0)
    {
      emit error(tr("Push has failed"));
    }
    else
    {
      emit pushed();
    }
  });

}

void GitInterface::pull(bool rebase)
{
  QList<QString> arguments = {"pull"};
  if (rebase)
  {
    arguments << "--rebase";
  }
  _impl->backgroundProcess->setArguments(arguments);
  _impl->backgroundProcess->start();

  _impl->callAsyncSingle(_impl->backgroundProcess, [=](int exitCode){
    if (exitCode != 0)
    {
      emit error(tr("Pull has failed"));
    }
    else
    {
      emit pulled();
    }
  });
}

void GitInterface::setFullFileDiff(bool fullFileDiff)
{
  _impl->fullFileDiff = fullFileDiff;
}
