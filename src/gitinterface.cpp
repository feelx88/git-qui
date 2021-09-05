#include "gitinterface.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QProcess>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

#include <iostream>

#define RUN_ONCE_IMPL(actionTag, type, runCall)                                \
  QFutureWatcher<type> *__watcher =                                            \
      new QFutureWatcher<type>(static_cast<QObject *>(this));                  \
  connect(__watcher, &QFutureWatcher<type>::finished, __watcher,               \
          &QFutureWatcher<type>::deleteLater);                                 \
  connect(__watcher, &QFutureWatcher<type>::finished, this,                    \
          std::bind(std::mem_fn(&GitInterfacePrivate::finishAction),           \
                    _impl.get(), actionTag));                                  \
  QFuture<type> __future = (runCall);                                          \
  __watcher->setFuture(__future);

#define RUN_ONCE_TYPED(actionTag, type, runCall)                               \
  if (!_impl->startAction(actionTag)) {                                        \
    return QFuture<type>();                                                    \
  }                                                                            \
  RUN_ONCE_IMPL(actionTag, type, runCall)

#define RUN_ONCE(actionTag, runCall)                                           \
  if (!_impl->startAction(actionTag)) {                                        \
    return;                                                                    \
  }                                                                            \
  RUN_ONCE_IMPL(actionTag, void, runCall)

struct GitProcess {
  int exitCode;
  QByteArray standardOutOutput, standardErrorOutput;
  QSharedPointer<QProcess> process;
};

class GitInterfacePrivate {
public:
  GitInterface *_this;
  QString name;
  QDir repositoryPath;
  bool readyForCommit = false;
  bool fullFileDiff = false;
  GitBranch activeBranch;
  QList<GitFile> files;
  QSharedPointer<QFile> errorLog;
  bool actionRunning = false;

  GitInterfacePrivate(GitInterface *_this) : _this(_this) {
    errorLog.reset(new QFile(GitInterface::errorLogFileName()));
    errorLog->open(QFile::Append);
  }

  GitProcess git(const QList<QString> &params, const QString &writeData = "") {
    QSharedPointer<QProcess> process = QSharedPointer<QProcess>::create();
    std::string stdWriteData = writeData.toStdString();

    process->setWorkingDirectory(repositoryPath.path());
    process->setProgram("git");
    process->setArguments(params);
    process->start();
    process->waitForStarted();
    process->write(stdWriteData.data(),
                   static_cast<qint64>(stdWriteData.length()));
    process->waitForBytesWritten();
    process->closeWriteChannel();
    process->waitForFinished();

    GitProcess gitProcess;
    gitProcess.exitCode = process->exitCode();
    gitProcess.standardOutOutput = process->readAllStandardOutput();
    gitProcess.standardErrorOutput = process->readAllStandardError();
    gitProcess.process = process;

    if (!gitProcess.standardErrorOutput.isEmpty()) {
      QString output = QString("[%1] %2").arg(
          QDateTime::currentDateTime().toString(Qt::ISODate),
          gitProcess.standardErrorOutput);
      errorLog->write(output.toLocal8Bit());
      emit _this->error(output, GitInterface::ActionTag::NO_TAG,
                        GitInterface::ErrorType::STDERR, true);
    }

    return gitProcess;
  }

  QString createPatch(const QList<GitDiffLine> &lines) {
    GitDiffLine first = lines.first();
    QString patch = first.header;
    QList<QList<GitDiffLine>> hunks;
    hunks.append(QList<GitDiffLine>());
    int lastindex = first.index;

    for (auto &line : lines) {
      if (!(line.index - lastindex <= 1)) {
        hunks.append(QList<GitDiffLine>());
      }

      hunks.last().append(line);
      lastindex = line.index;
    }

    for (auto hunk : hunks) {
      int newCount = 0, oldCount = 0;
      GitDiffLine first = hunk.first();

      for (GitDiffLine &line : hunk) {
        if (line.type == GitDiffLine::diffType::ADD) {
          ++newCount;
        } else if (line.type == GitDiffLine::diffType::REMOVE ||
                   line.type == GitDiffLine::diffType::CONTEXT) {
          ++oldCount;
        }
      }

      if (first.oldLine < 0) {
        first.oldLine = first.newLine;
      }

      patch.append(QString::asprintf("@@ -%i,%i +%i,%i @@\n", first.oldLine,
                                     oldCount, first.oldLine, newCount));

      for (GitDiffLine &line : hunk) {
        if (line.type == GitDiffLine::diffType::ADD) {
          patch += "+";
        } else if (line.type == GitDiffLine::diffType::REMOVE) {
          patch += "-";
        }
        patch += line.content.remove('\n') + '\n';
      }
    }

    return patch;
  }

  bool startAction(const GitInterface::ActionTag &actionTag) {
    if (actionRunning) {
      emit _this->error(QObject::tr("Already running"), actionTag,
                        GitInterface::ErrorType::ALREADY_RUNNING);
      return false;
    }
    actionRunning = true;
    emit _this->actionStarted(actionTag);
    return true;
  }

  void finishAction(const GitInterface::ActionTag &actionTag) {
    actionRunning = false;
    emit _this->actionFinished(actionTag);
  }

  QList<GitBranch> branches(const QStringList &args) {
    auto process = git({"remote"});
    QList<QByteArray> remotes = process.standardOutOutput.split('\n');
    remotes.pop_back();

    process = git(QList<QString>{"branch", "--format="
                                           "%(HEAD)"
                                           "#"
                                           "%(refname:short)"
                                           "#"
                                           "%(upstream:short)"}
                  << args);

    QList<GitBranch> branches;

    for (auto &line : process.standardOutOutput.split('\n')) {
      if (!line.isEmpty()) {
        auto parts = line.split('#');

        if (parts.at(1).endsWith("HEAD")) {
          continue;
        }

        branches.append({parts[0] == "*", parts[1], parts[2],
                         remotes.indexOf(parts[1].split('/')[0]) > -1});
      }
    }

    return branches;
  }

  void reload() {
    status();
    log();
    emit _this->reloaded();
  }

  void status() {
    auto process = git({
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

    for (auto &output : process.standardOutOutput.split('\0')) {
      if (output.isEmpty() || !output.contains(' ')) {
        continue;
      }

      if (output.startsWith("##")) {
        QRegExp branchRegex(
            "## (.*)\\.\\.\\..*(?:ahead ([0-9]+))?.*(?:behind ([0-9]+))?.*");
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

      switch (firstByte) {
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

      switch (secondByte) {
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

      if (file.unstaged) {
        unstaged.append(file);
      }

      if (file.staged) {
        staged.append(file);
      }
    }

    readyForCommit = !staged.empty();

    emit _this->nonStagingAreaChanged(unstaged);
    emit _this->stagingAreaChanged(staged);

    files.clear();
    files << unstaged << staged;

    QList<GitBranch> branches = this->branches({"--all"});

    for (auto &branch : branches) {
      if (branch.active) {
        activeBranch = branch;
        break;
      }
    }

    emit _this->branchesChanged(branches);
    emit _this->branchChanged(branchName, !(unstaged.empty() && staged.empty()),
                              hasUpstream, commitsAhead, commitsBehind);
  }

  void log() {
    auto process = git({"log", "--all", "--full-history",
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
                        "%D"});

    QList<GitCommit> list;

    for (auto &line : QString(process.standardOutOutput).split('\n')) {
      if (line.isEmpty()) {
        continue;
      }

      QList<QString> parts = line.split('\f');

      GitCommit commit;
      if (parts.length() > 1) {
        commit.id = parts.at(1);
        commit.message = parts.at(2);
        commit.author = parts.at(3);
        commit.date = QDateTime::fromSecsSinceEpoch(parts.at(4).toInt());
        commit.branches = parts.at(5).split(", ", Qt::SkipEmptyParts);
      }
      list.append(commit);
    }

    emit _this->logChanged(list);
  }

  void fetch() {
    git({"fetch", "--all", "--prune"});
    _this->reload();
  }

  void pull(bool rebase) {
    QList<QString> arguments = {"pull"};
    if (rebase) {
      arguments << "--rebase";
    }
    auto process = git(arguments);

    status();

    if (process.exitCode != EXIT_SUCCESS) {
      emit _this->error(QObject::tr("Pull has failed"),
                        GitInterface::ActionTag::GIT_PULL,
                        GitInterface::ErrorType::GENERIC);
    }
  }
};

GitInterface::GitInterface(const QString &name, const QString &path,
                           QObject *parent)
    : QObject(parent), _impl(new GitInterfacePrivate(this)) {
  _impl->name = name;
  _impl->repositoryPath.setPath(path);
  reload();
}

GitInterface::~GitInterface() {}

const QString GitInterface::name() const { return _impl->name; }

void GitInterface::setName(const QString &name) { _impl->name = name; }

const QString GitInterface::path() const {
  return _impl->repositoryPath.path();
}

void GitInterface::setPath(const QString &path) {
  _impl->repositoryPath.setPath(path);
}

const GitBranch GitInterface::activeBranch() const {
  return _impl->activeBranch;
}

bool GitInterface::actionRunning() const { return _impl->actionRunning; }

QFuture<QList<GitBranch>> GitInterface::branches(const QList<QString> &args) {
  RUN_ONCE_TYPED(
      ActionTag::GIT_BRANCH, QList<GitBranch>,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::branches, args));

  return __future;
}

const QList<GitFile> GitInterface::files() const { return _impl->files; }

QString GitInterface::errorLogFileName() {
  return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
         "-error.log";
}

void GitInterface::reload() { _impl->reload(); }

void GitInterface::status() {
  QtConcurrent::run(_impl.get(), &GitInterfacePrivate::status);
}

void GitInterface::log() {
  QtConcurrent::run(_impl.get(), &GitInterfacePrivate::log);
}

void GitInterface::fetch() {
  QtConcurrent::run(_impl.get(), &GitInterfacePrivate::fetch);
}

bool GitInterface::commit(const QString &message) {
  if (!_impl->readyForCommit) {
    emit error(tr("There are no files to commit"), ActionTag::GIT_COMMIT,
               ErrorType::GENERIC);
    return false;
  }

  auto process = _impl->git({
      "commit",
      "--message",
      message,
  });

  if (process.exitCode != EXIT_SUCCESS) {
    emit error(process.standardErrorOutput, ActionTag::GIT_COMMIT,
               ErrorType::GENERIC);
    return false;
  }

  reload();
  emit commited();
  return true;
}

void GitInterface::stageFile(const QString &path) {
  _impl->git({"add", path});
  status();
}

void GitInterface::unstageFile(const QString &path) {
  _impl->git({"reset", "HEAD", path});
  status();
}

void GitInterface::selectFile(bool unstaged, const QString &path) {
  emit fileSelected(unstaged, path);

  diffFile(unstaged, path);
}

void GitInterface::diffFile(bool unstaged, const QString &path) {
  QString absolutePath = _impl->repositoryPath.absolutePath() + "/" + path;

  if (path.isEmpty() || !QFileInfo(absolutePath).isFile()) {
    return;
  }

  bool binary = false;
  int lineCount = 0;

  QFile *file = new QFile(absolutePath, this);
  file->open(QIODevice::ReadOnly);

  QByteArray tmp = file->read(1024 * 512); // 512kiB
  if (tmp.size() > 0 && tmp.contains('\0')) {
    binary = true;
  } else {
    file->reset();
    while (!file->readLine(0).isEmpty()) {
      ++lineCount;
    }
  }

  QList<GitDiffLine> list;
  GitProcess process;

  if (unstaged) {
    process = _impl->git(
        {"diff", _impl->fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
         "--", path});
  } else {
    process = _impl->git(
        {"diff", _impl->fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
         "HEAD", "--cached", "--", path});
  }

  QTextStream stream(&process.standardOutOutput);

  QString output = stream.readLine();
  QRegExp regex("@* \\-(\\d+),.* \\+(\\d+),.*");
  QStringList lineNos;
  int index = 0;

  int lineNoOld = -1;
  int lineNoNew = -1;

  auto readLine = std::function<QString()>([&] { return stream.readLine(); });

  if (output.length() == 0) {
    lineNoOld = -1;
    lineNoNew = 1;

    GitDiffLine header;
    header.type = GitDiffLine::diffType::HEADER;
    header.content = "untracked file";

    list.append(header);

    file->reset();

    readLine = std::function<QByteArray()>([&] {
      QByteArray arr(file->readLine());
      if (arr.size() > 0) {
        arr.prepend("+ ");
      }
      return arr;
    });

    output = readLine();
  }

  QString header;

  if (binary) {
    GitDiffLine line;
    line.type = GitDiffLine::diffType::HEADER;
    line.content = "binary file";
    list.append(line);
  } else {
    while (output.length() > 0) {
      GitDiffLine line;
      line.oldLine = lineNoOld;
      line.newLine = lineNoNew;
      line.index = index++;

      while (output.length() > 2 &&
             (output.endsWith(' ') || output.endsWith('\n') ||
              output.endsWith('\t'))) {
        output.chop(1);
      }
      line.content = output;

      line.type = GitDiffLine::diffType::FILE_HEADER;

      if (output.startsWith("index") || output.startsWith("diff") ||
          output.startsWith("+++") || output.startsWith("---") ||
          output.startsWith("new file") || output.startsWith("old file")) {
        line.type = GitDiffLine::diffType::HEADER;
        header += output + '\n';
      } else {
        line.header = header;
        switch (output.at(0).toLatin1()) {
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

void GitInterface::addLines(const QList<GitDiffLine> &lines, bool unstage) {
  if (lines.isEmpty()) {
    return;
  }

  QString patch = _impl->createPatch(lines);
  qDebug().noquote() << patch;

  if (unstage) {
    _impl->git(
        {"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "-"},
        patch);
  } else {
    _impl->git({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn",
                "--reverse", "-"},
               patch);
  }

  status();
}

void GitInterface::push(const QString &remote, const QVariant &branch,
                        bool setUpstream) {
  emit pushStarted();

  QList<QString> args = {"push", remote};

  if (setUpstream) {
    args << "--set-upstream";
  }

  if (!branch.isNull()) {
    args << branch.toString();
  }

  auto process = _impl->git(args);

  status();
  emit pushed();

  if (process.exitCode != EXIT_SUCCESS) {
    emit error(tr("Push has failed"), ActionTag::GIT_PUSH, ErrorType::GENERIC);
  }
}

void GitInterface::pull(bool rebase) {
  RUN_ONCE(ActionTag::GIT_PULL,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::pull, rebase));
}

void GitInterface::setFullFileDiff(bool fullFileDiff) {
  _impl->fullFileDiff = fullFileDiff;
}

void GitInterface::revertLastCommit() {
  auto process = _impl->git({"log", "--max-count=1",
                             "--pretty="
                             "%s"});

  QString message = process.standardOutOutput;

  _impl->git({"reset", "--soft", "HEAD^"});

  reload();
  emit lastCommitReverted(message);
}

void GitInterface::resetLines(const QList<GitDiffLine> &lines) {
  if (lines.empty()) {
    return;
  }

  QString patch = _impl->createPatch(lines);
  qDebug().noquote() << patch;

  _impl->git(
      {"apply", "--reverse", "--unidiff-zero", "--whitespace=nowarn", "-"},
      patch);

  status();
}

void GitInterface::checkoutPath(const QString &path) {
  auto process = _impl->git({"status", "--porcelain=v1", path});
  QString fileStatus = process.standardOutOutput;

  if (fileStatus.startsWith("??")) {
    QFile::remove(_impl->repositoryPath.filePath(path));
  } else {
    _impl->git({"checkout", "--", path});
  }

  status();
}

void GitInterface::changeBranch(const QString &branchName,
                                const QString &upstreamBranchName) {
  if (upstreamBranchName.isEmpty()) {
    _impl->git({"checkout", branchName});
  } else {
    _impl->git({"checkout", "-b", branchName, upstreamBranchName});
  }
  reload();
}

void GitInterface::createBranch(const QString &name) {
  _impl->git({"branch", name});
  status();
}

void GitInterface::deleteBranch(const QString &name) {
  _impl->git({"branch", "-d", name});
  status();
}

void GitInterface::setUpstream(const QString &remote, const QString &branch) {
  _impl->git(
      {"branch", QString("--set-upstream-to=%1/%2").arg(remote, branch)});
}

void GitInterface::stash() {
  _impl->git({"stash", "--include-untracked"});
  status();
}

void GitInterface::stashPop() {
  _impl->git({"stash", "pop"});
  status();
}
