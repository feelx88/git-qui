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

  void connect()
  {
    QObject::connect(process, static_cast<void(QProcess::*)(int)>(&QProcess::finished), process, [=](int){
      auto output = process->readAllStandardError();
      if (!output.isEmpty())
      {
        qDebug() << output;
      }
    });
  }

  bool readyForCommit = false;
};

GitInterface::GitInterface(QObject *parent, const QString &path)
  : QObject(parent),
    _impl(new GitInterfacePrivate)
{
  _impl->process = new QProcess(this);
  _impl->process->setProgram("git");

  _impl->connect();

  switchRepository(path);
}

GitInterface::~GitInterface()
{
}

void GitInterface::switchRepository(const QString &path)
{
  _impl->repositoryPath = path;
  _impl->process->setWorkingDirectory(path);
  emit repositorySwitched(path);
  reload();
}

void GitInterface::reload()
{
  status();
  log();
  emit reloaded();
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

  _impl->readyForCommit = !staged.empty();

  emit nonStagingAreaChanged(unstaged);
  emit stagingAreaChanged(staged);

  _impl->process->setArguments({
    "rev-parse",
    "--abbrev-ref",
    "HEAD"
  });

  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  emit branchChanged(_impl->process->readAll().trimmed(), !(unstaged.empty() && staged.empty()));
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
  if (!_impl->readyForCommit)
  {
    emit error(tr("There are no files to commit"));
    return;
  }

  _impl->process->setArguments({"commit",
                                  "--message",
                                  message,
                                 });
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  reload();
  emit commited();
}

void GitInterface::stageFile(const QString &path)
{
  _impl->process->setArguments({"add", path});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  status();
}

void GitInterface::unstageFile(const QString &path)
{
  _impl->process->setArguments({"reset", "HEAD", path});
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

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

  QList<GitDiffLine> list;

  if(unstaged)
  {
    _impl->process->setArguments({"diff", "--", path});
  }
  else
  {
    _impl->process->setArguments({"diff", "HEAD", "--cached", "--", path});
  }
  _impl->process->start(QIODevice::ReadOnly);
  _impl->process->waitForFinished();

  QByteArray output = _impl->process->readLine();
  QRegExp regex("@* \\-(\\d+),.* \\+(\\d+),.*");
  QStringList lineNos;
  int index = 0;

  int lineNoOld = -1;
  int lineNoNew = -1;

  bool binary = false;

  QFile *file = new QFile(absolutePath, this);
  file->open(QIODevice::ReadOnly);

  QByteArray tmp = file->read(1024 * 512); //512kiB
  if(tmp.size() > 0 && tmp.contains('\0'))
  {
    binary = true;
  }

  auto readLine = new std::function<QByteArray()>([&] {
    return _impl->process->readLine();
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
    _impl->process->setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "-"});
  }
  else
  {
    _impl->process->setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "--reverse", "-"});
  }

  qDebug().noquote() << patch;

  std::string stdPatch = patch.toStdString();
  _impl->process->start(QIODevice::ReadWrite);
  _impl->process->waitForStarted();
  _impl->process->write(stdPatch.data(), stdPatch.length());
  _impl->process->closeWriteChannel();
  _impl->process->waitForFinished();

  status();
}
