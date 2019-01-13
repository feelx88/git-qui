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
  bool readyForCommit = false;
  bool fullFileDiff = false;

  QSharedPointer<QProcess> git(const QList<QString> &params)
  {
    QSharedPointer<QProcess> process = QSharedPointer<QProcess>::create();

    auto logger = [=](int){
      auto output = process->readAllStandardError();
      if (!output.isEmpty())
      {
        qDebug() << output;
      }
    };
    process->connect(process.get(), static_cast<void(QProcess::*)(int)>(&QProcess::finished), process.get(), logger);

    process->setWorkingDirectory(repositoryPath.path());
    process->setProgram("git");
    process->setArguments(params);
    process->start();
    process->waitForFinished();

    return process;
  }

  QString createPatch(const QList<GitDiffLine> &lines)
  {
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

    return patch;
  }
};

GitInterface::GitInterface(QObject *parent, const QString &path)
  : QObject(parent),
    _impl(new GitInterfacePrivate)
{
  _impl->repositoryPath = path;
  reload();
}

GitInterface::~GitInterface()
{
}

const QString GitInterface::path()
{
  return _impl->repositoryPath.path();
}

void GitInterface::reload()
{
  status();
  log();
  emit reloaded();
}

void GitInterface::status()
{
  auto process = _impl->git({
                              "status",
                              "--untracked=all",
                              "--porcelain=v1",
                              "-b",
                              "-z",
                            });

  QList<GitFile> unstaged, staged;
  QString branchName;
  bool hasUpstream = false;
  int commitsAhead = 0, commitsBehind = 0;

  for(auto output : process->readAll().split('\0'))
  {
    if(output.isEmpty() || !output.contains(' '))
    {
      continue;
    }

    if (output.startsWith("##"))
    {
      QRegExp branchRegex("## (.*)\\.\\.\\..*(?:ahead ([0-9]+))?.*(?:behind ([0-9]+))?.*");
      hasUpstream = branchRegex.indexIn(output) > -1;
      branchName = hasUpstream ? branchRegex.cap(1) : output.split(' ').at(1);
      commitsAhead = branchRegex.cap(2).toInt();
      commitsBehind = branchRegex.cap(3).toInt();
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
  emit branchChanged(branchName, !(unstaged.empty() && staged.empty()), hasUpstream, commitsAhead, commitsBehind);
}

void GitInterface::log()
{
  auto process = _impl->git({"log",
                             "--all",
                             "--full-history",
                             "--pretty="
                             "%x0c"
                             "%h"
                             "%x0c"
                             "%s"
                             "%x0c"
                             "%an"
                             "%x0c"
                             "%ct"
                             "%x0c"
                             "%D"
                            });

  QList<GitCommit> list;

  for (auto line : QString(process->readAllStandardOutput()).split('\n'))
  {
    if (line.isEmpty())
    {
      continue;
    }

    QList<QString> parts = line.split('\f');

    GitCommit commit;
    if (parts.length() > 1)
    {
      commit.id = parts.at(1);
      commit.message = parts.at(2);
      commit.author = parts.at(3);
      commit.date = QDateTime::fromSecsSinceEpoch(parts.at(4).toInt());
      commit.branches = parts.at(5).split(", ", QString::SkipEmptyParts);
    }
    list.append(commit);
  }

  emit logChanged(list);
}

void GitInterface::fetch()
{
  _impl->git({"fetch", "--all"});
  reload();
}

void GitInterface::commit(const QString &message)
{
  if (!_impl->readyForCommit)
  {
    emit error(tr("There are no files to commit"));
    return;
  }

  auto process = _impl->git({"commit",
                             "--message",
                             message,
                            });
  reload();
  emit commited();
}

void GitInterface::stageFile(const QString &path)
{
  _impl->git({"add", path});
  status();
}

void GitInterface::unstageFile(const QString &path)
{
  _impl->git({"reset", "HEAD", path});
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
  QSharedPointer<QProcess> process;

  if(unstaged)
  {
    process = _impl->git({
                           "diff",
                           _impl->fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
                           "--",
                           path
                         });
  }
  else
  {
    process = _impl->git({
                           "diff",
                           _impl->fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
                           "HEAD",
                           "--cached",
                           "--",
                           path
                         });
  }

  QByteArray output = process->readLine();
  QRegExp regex("@* \\-(\\d+),.* \\+(\\d+),.*");
  QStringList lineNos;
  int index = 0;

  int lineNoOld = -1;
  int lineNoNew = -1;

  auto readLine = std::function<QByteArray()>([&] {
    return process->readLine();
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

    readLine = std::function<QByteArray()>([&] {
      QByteArray arr(file->readLine());
      if (arr.size() > 0)
      {
        arr.prepend("+ ");
      }
      return arr;
    });

    output = readLine();
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
      output = readLine();
    }
  }

  emit fileDiffed(path, list, unstaged);
}

void GitInterface::addLines(const QList<GitDiffLine> &lines, bool unstage)
{
  if (lines.isEmpty())
  {
    return;
  }

  QString patch =_impl->createPatch(lines);

  qDebug().noquote() << patch;

  QProcess process;

  if(unstage)
  {
    process.setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "-"});
  }
  else
  {
    process.setArguments({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "--reverse", "-"});
  }


  std::string stdPatch = patch.toStdString();
  process.start(QIODevice::ReadWrite);
  process.waitForStarted();
  process.write(stdPatch.data(), stdPatch.length());
  process.closeWriteChannel();
  process.waitForFinished();

  status();
}

void GitInterface::push()
{
  auto process = _impl->git({
                              "push",
                              "origin",
                              "HEAD"
                            });

  if (process->exitCode() != 0)
  {
    emit error(tr("Push has failed"));
  }
  else
  {
    status();
    emit pushed();
  }
}

void GitInterface::pull(bool rebase)
{
  QList<QString> arguments = {"pull"};
  if (rebase)
  {
    arguments << "--rebase";
  }
  auto process = _impl->git(arguments);

  if (process->exitCode() != 0)
  {
    emit error(tr("Pull has failed"));
  }
  else
  {
    status();
    emit pulled();
  }
}

void GitInterface::setFullFileDiff(bool fullFileDiff)
{
  _impl->fullFileDiff = fullFileDiff;
}

void GitInterface::revertLastCommit()
{
  auto process = _impl->git({
                              "log",
                              "--max-count=1",
                              "--pretty="
                              "%s"
                            });

  QString message = process->readAllStandardOutput();

  _impl->git({
               "reset",
               "--soft",
               "HEAD^"
             });

  reload();
  emit lastCommitReverted(message);
}

void GitInterface::resetLines(const QList<GitDiffLine> &lines)
{
  if (lines.empty())
  {
    return;
  }

  QString patch = _impl->createPatch(lines);

  qDebug().noquote() << patch;
  QProcess process;

  process.setArguments({"apply", "--reverse", "--unidiff-zero", "--whitespace=nowarn", "-"});

  std::string stdPatch = patch.toStdString();
  process.start(QIODevice::ReadWrite);
  process.waitForStarted();
  process.write(stdPatch.data(), stdPatch.length());
  process.closeWriteChannel();
  process.waitForFinished();

  status();
}

void GitInterface::checkoutPath(const QString &path)
{
  _impl->git({"checkout", "--", path});

  status();
}
