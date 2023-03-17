#include "gitinterface.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QProcess>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

#include "gitcommit.hpp"

#define WATCH_ASYNC_METHOD_CALL_TYPED(actionTag, type, runCall)                \
  QFutureWatcher<type> *__watcher =                                            \
      new QFutureWatcher<type>(static_cast<QObject *>(this));                  \
  connect(__watcher, &QFutureWatcher<type>::finished, __watcher,               \
          &QFutureWatcher<type>::deleteLater);                                 \
  connect(__watcher, &QFutureWatcher<type>::finished, this,                    \
          [=, this] { _impl->finishAction(actionTag); });                      \
  QFuture<type> __future;                                                      \
  __future = (runCall);                                                        \
  __watcher->setFuture(__future);

#define WATCH_ASYNC_METHOD_CALL(actionTag, runCall)                            \
  WATCH_ASYNC_METHOD_CALL_TYPED(actionTag, void, runCall);

#define RUN_ONCE_TYPED(actionTag, type, runCall)                               \
  if (!_impl->startAction(actionTag)) {                                        \
    return QFuture<type>();                                                    \
  }                                                                            \
  WATCH_ASYNC_METHOD_CALL_TYPED(actionTag, type, runCall);                     \
  _impl->actionFuture = __future;                                              \
  return __future;

#define RUN_ONCE(actionTag, runCall) RUN_ONCE_TYPED(actionTag, void, runCall)

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
  QList<GitBranch> branches;
  QList<GitFile> files, unstagedFiles, stagedFiles;
  QSharedPointer<QFile> errorLog;
  QFuture<void> actionFuture;
  QByteArray branchesHash, unstagedFilesHash, stagedFilesHash;
  QList<QString> remotes;

  GitInterfacePrivate(GitInterface *_this) : _this(_this) {
    errorLog.reset(new QFile(GitInterface::errorLogFileName()));
    errorLog->open(QFile::Append);
  }

  bool startAction(const GitInterface::ActionTag &actionTag) {
    if (actionFuture.isRunning()) {
      emit _this->error(QObject::tr("Already running"), actionTag,
                        GitInterface::ErrorType::ALREADY_RUNNING);
      return false;
    }
    emit _this->actionStarted(actionTag);
    return true;
  }

  void finishAction(const GitInterface::ActionTag &actionTag) {
    emit _this->actionFinished(actionTag);
  }

  GitProcess git(const QList<QString> &params, const QString &writeData = "") {
    QSharedPointer<QProcess> process = QSharedPointer<QProcess>::create();
    std::string stdWriteData = writeData.toStdString();

#ifdef FLATPAK_BUILD
    process->setProgram("flatpak-spawn");
    process->setArguments(
        QStringList{"--host",
                    QString("--directory=%1").arg(repositoryPath.path()), "git"}
        << params);
#else
    process->setWorkingDirectory(repositoryPath.path());
    process->setProgram("git");
    process->setArguments(params);
#endif
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

  QList<GitBranch> branch(const QStringList &args) {
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

    branches.clear();

    for (auto &line : process.standardOutOutput.split('\n')) {
      if (!line.isEmpty()) {
        auto parts = line.split('#');

        if (parts.at(1).endsWith("HEAD")) {
          continue;
        }

        branches.append({parts[0] == "*",
                         remotes.indexOf(parts[1].split('/')[0]) > -1, false,
                         false, parts[1], parts[2], 0, 0});
      }
    }

    return branches;
  }

  void reload() {
    status();
    log();
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
      case '?':
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

    QCryptographicHash unstagedHash(QCryptographicHash::Sha256);
    for (auto &file : qAsConst(unstaged)) {
      if (file.deleted) {
        unstagedHash.addData(
            QByteArray(QString("DELETED %1").arg(file.path).toLatin1()));
      } else {
        QFile f(repositoryPath.absoluteFilePath(file.path));
        f.open(QFile::ReadOnly);
        unstagedHash.addData(f.readAll());
        f.close();
      }
    }

    if (unstaged.empty() || unstagedHash.result() != unstagedFilesHash) {
      unstagedFilesHash = unstagedHash.result();
      unstagedFiles = unstaged;
      emit _this->nonStagingAreaChanged(unstaged);
    }

    QCryptographicHash stagedHash(QCryptographicHash::Sha256);
    for (auto &file : qAsConst(staged)) {
      if (file.deleted) {
        stagedHash.addData(
            QByteArray(QString("DELETED %1").arg(file.path).toLatin1()));
      } else {
        QFile f(repositoryPath.absoluteFilePath(file.path));
        f.open(QFile::ReadOnly);
        stagedHash.addData(f.readAll());
        f.close();
      }
    }

    if (staged.empty() || stagedHash.result() != stagedFilesHash) {
      stagedFilesHash = stagedHash.result();
      stagedFiles = staged;
      emit _this->stagingAreaChanged(staged);
    }

    files.clear();
    files << unstaged << staged;

    QList<GitBranch> branches = this->branch({"--all"});

    QCryptographicHash localBranchesHash(QCryptographicHash::Sha256);
    for (auto &branch : branches) {
      localBranchesHash.addData(branch.name.toLatin1());
      if (branch.active) {
        activeBranch = branch;
        activeBranch.hasChanges = !(unstaged.empty() && staged.empty());
        activeBranch.hasUpstream = hasUpstream;
        activeBranch.commitsAhead = commitsAhead;
        activeBranch.commitsBehind = commitsBehind;
        localBranchesHash.addData("*");
      }
    }

    if (localBranchesHash.result() != branchesHash) {
      branchesHash = localBranchesHash.result();
      emit _this->branchesChanged(branches);
      emit _this->branchChanged(activeBranch);
    }

    process = git({"remote"});

    remotes.clear();
    for (auto &output : process.standardOutOutput.split('\n')) {
      remotes.append(output);
    }
  }

  void log() {
    auto process =
        git({"log", "--all", "--full-history", "--parents", "--topo-order",
             "--pretty="
             "%x0c"
             "%H"
             "%x0c"
             "%s"
             "%x0c"
             "%an"
             "%x0c"
             "%ct"
             "%x0c"
             "%D"
             "%x0c"
             "%P"});

    QList<GitCommit> list;

    for (auto &line : QString(process.standardOutOutput).split('\n')) {
      if (line.isEmpty()) {
        continue;
      }

      QList<QString> parts = line.split('\f');
      QList<QString> remoteParts = remotes;
      for (auto &remote : remoteParts) {
        remote.append('/');
      }

      GitCommit commit;
      if (parts.length() > 1) {
        commit.id = parts.at(1);
        commit.message = parts.at(2);
        commit.author = parts.at(3);
        commit.date = QDateTime::fromSecsSinceEpoch(parts.at(4).toInt());
        QList<QString> refs = parts.at(5).split(", ", Qt::SkipEmptyParts);
        commit.parentIds = parts.at(6).split(" ", Qt::SkipEmptyParts);

        for (int refIndex = 0; refIndex < refs.size(); ++refIndex) {
          GitRef ref;
          ref.name = refs.at(refIndex);
          if (ref.name.startsWith("HEAD ->")) {
            ref.name.remove("HEAD -> ");
            commit.isHead = true;
            ref.isHead = true;
          }

          if (ref.name.startsWith("tag:")) {
            ref.name.remove("tag: ");
            ref.isTag = true;
          }

          for (const auto &remote : remoteParts) {
            if (ref.name.startsWith(remote)) {
              ref.remotePart = remote;
              ref.isRemote = true;
              break;
            }
          }

          commit.refs.append(ref);
        }
      }

      list.append(commit);
    }

    QSharedPointer<GitTree> tree(new GitTree(list));
    emit _this->logChanged(tree);
  }

  void fetch() {
    git({"fetch", "--all", "--prune"});
    reload();
  }

  bool commit(const QString &message) {
    if (!readyForCommit) {
      emit _this->error(QObject::tr("There are no files to commit"),
                        GitInterface::ActionTag::GIT_COMMIT,
                        GitInterface::ErrorType::GENERIC);
      return false;
    }

    auto process = git({
        "commit",
        "--message",
        message,
    });

    if (process.exitCode != EXIT_SUCCESS) {
      emit _this->error(process.standardErrorOutput,
                        GitInterface::ActionTag::GIT_COMMIT,
                        GitInterface::ErrorType::GENERIC);
      return false;
    }

    reload();
    return true;
  }

  void stageFiles(const QStringList &paths) {
    for (auto &path : paths) {
      git({"add", path});
    }
    status();
  }

  void unstageFile(const QString &path) {
    git({"reset", "HEAD", path});
    status();
  }

  void diffFile(bool unstaged, const QString &path,
                const QString &commitId = QString()) {
    QString absolutePath = repositoryPath.absolutePath() + "/" + path;

    if (path.isEmpty() || !QFileInfo(absolutePath).isFile()) {
      return;
    }

    bool binary = false;
    int lineCount = 0;

    QFile *file = new QFile(absolutePath, _this);
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

    if (!commitId.isNull()) {
      process =
          git({"diff", fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
               QString("%1~1..%1").arg(commitId), "--", path});
    } else if (unstaged) {
      process =
          git({"diff", fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
               "--", path});
    } else {
      process =
          git({"diff", fullFileDiff ? QString("-U%1").arg(lineCount) : "-U3",
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

    file->deleteLater();

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

    if (!commitId.isNull()) {
      emit _this->historyFileDiffed(path, list, commitId);
    } else {
      emit _this->fileDiffed(path, list, unstaged);
    }
  }

  void historyDiffFile(const QString &commitId, const QString &path) {}

  void addLines(const QList<GitDiffLine> &lines, bool unstage) {
    if (lines.isEmpty()) {
      return;
    }

    QString patch = createPatch(lines);
    qDebug().noquote() << patch;

    if (unstage) {
      git({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn", "-"},
          patch);
    } else {
      git({"apply", "--cached", "--unidiff-zero", "--whitespace=nowarn",
           "--reverse", "-"},
          patch);
    }

    status();
  }

  void push(const QString &remote, const QVariant &branch, bool setUpstream) {
    QList<QString> args = {"push", remote};

    if (setUpstream) {
      args << "--set-upstream";
    }

    if (!branch.isNull()) {
      args << branch.toString();
    }

    auto process = git(args);
    if (process.exitCode != EXIT_SUCCESS) {
      emit _this->error(QObject::tr("Push has failed"),
                        GitInterface::ActionTag::GIT_PUSH,
                        GitInterface::ErrorType::GENERIC);
    }

    status();
    log();
  }

  void pushTags(const QString &remote) {
    QList<QString> args = {"push", "--tags", remote};

    auto process = git(args);
    if (process.exitCode != EXIT_SUCCESS) {
      emit _this->error(QObject::tr("Push tags has failed"),
                        GitInterface::ActionTag::GIT_PUSH,
                        GitInterface::ErrorType::GENERIC);
    }

    status();
    log();
  }

  void pull(bool rebase) {
    QList<QString> arguments = {"pull"};
    if (rebase) {
      arguments << "--rebase";
    }
    auto process = git(arguments);

    status();
    log();

    if (process.exitCode != EXIT_SUCCESS) {
      emit _this->error(QObject::tr("Pull has failed"),
                        GitInterface::ActionTag::GIT_PULL,
                        GitInterface::ErrorType::GENERIC);
    }
  }

  void revertLastCommit() {
    auto process = git({"log", "--max-count=1",
                        "--pretty="
                        "%s"});

    QString message = process.standardOutOutput;

    git({"reset", "--soft", "HEAD^"});

    reload();
    emit _this->lastCommitReverted(message);
  }

  void resetLines(const QList<GitDiffLine> &lines) {
    if (lines.empty()) {
      return;
    }

    QString patch = createPatch(lines);
    qDebug().noquote() << patch;

    git({"apply", "--reverse", "--unidiff-zero", "--whitespace=nowarn", "-"},
        patch);

    status();
  }

  void checkoutPath(const QString &path) {
    auto process = git({"status", "--porcelain=v1", path});
    QString fileStatus = process.standardOutOutput;

    if (fileStatus.startsWith("??")) {
      QFile::remove(repositoryPath.filePath(path));
    } else {
      git({"checkout", "--", path});
    }

    status();
  }

  void changeBranch(const QString &branchName,
                    const QString &upstreamBranchName) {
    if (upstreamBranchName.isEmpty()) {
      git({"checkout", branchName});
    } else {
      git({"checkout", "-b", branchName, upstreamBranchName});
    }
    reload();
  }

  void createBranch(const QString &name, const QString &baseCommit) {
    QList<QString> arguments = {"branch", name};
    if (!baseCommit.isEmpty()) {
      arguments << baseCommit;
    }
    git(arguments);
    status();
    log();
  }

  void deleteBranch(const QString &name) {
    git({"branch", "-d", name});
    status();
    log();
  }

  void setUpstream(const QString &remote, const QString &branch) {
    git({"branch", QString("--set-upstream-to=%1/%2").arg(remote, branch)});
  }

  void createTag(const QString &name, const QString &commitId) {
    git({"tag", name, commitId});
    log();
  }

  void deleteTag(const QString &name) {
    git({"tag", "-d", name});
    log();
  }

  void stash() {
    git({"stash", "--include-untracked"});
    status();
    log();
  }

  void stashPop() {
    git({"stash", "pop"});
    status();
    log();
  }

  void resetToCommit(const QString &commitId,
                     const GitInterface::ResetType &type) {
    QString typeArg;
    switch (type) {
    case GitInterface::ResetType::MIXED:
      typeArg = "--mixed";
      break;
    case GitInterface::ResetType::SOFT:
      typeArg = "--soft";
      break;
    case GitInterface::ResetType::HARD:
      typeArg = "--hard";
      break;
    }

    git({"reset", typeArg, commitId});
    status();
    log();
  }

  void cherryPickCommit(const QString &commitId, const QStringList &extraArgs) {
    git(QStringList{"cherry-pick"} << extraArgs << commitId);

    log();
  }
};

GitInterface::GitInterface(const QString &name, const QString &path,
                           QObject *parent)
    : QObject(parent), _impl(new GitInterfacePrivate(this)) {
  _impl->name = name;
  _impl->repositoryPath.setPath(path);
  reload();
}

GitInterface::~GitInterface() {
  _impl->actionFuture.cancel();
  _impl->actionFuture.waitForFinished();
}

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

bool GitInterface::actionRunning() const {
  return _impl->actionFuture.isRunning();
}

QFuture<QList<GitBranch>> GitInterface::branch(const QList<QString> &args) {
  RUN_ONCE_TYPED(
      ActionTag::GIT_BRANCH, QList<GitBranch>,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::branch, args));
}

const QList<GitFile> GitInterface::files() const { return _impl->files; }

const QList<GitFile> GitInterface::unstagedFiles() const {
  return _impl->unstagedFiles;
}

const QList<GitFile> GitInterface::stagedFiles() const {
  return _impl->stagedFiles;
}

const QList<GitBranch> GitInterface::branches() const {
  return _impl->branches;
}

const QList<QString> GitInterface::remotes() const { return _impl->remotes; }

QString GitInterface::errorLogFileName() {
  return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
         "-error.log";
}

void GitInterface::setFullFileDiff(bool fullFileDiff) {
  _impl->fullFileDiff = fullFileDiff;
}

void GitInterface::fetchNonBlocking() {
  emit actionStarted(ActionTag::GIT_FETCH);
  WATCH_ASYNC_METHOD_CALL(
      ActionTag::GIT_FETCH,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::fetch));
}

QFuture<void> GitInterface::reload() {
  RUN_ONCE(ActionTag::RELOAD,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::reload));
}

void GitInterface::status() {
  emit actionStarted(ActionTag::GIT_STATUS);
  WATCH_ASYNC_METHOD_CALL(
      ActionTag::GIT_STATUS,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::status));
}

void GitInterface::historyStatus(const QString &commitId) {
  auto process =
      _impl->git({"diff", "-z", "--name-only", commitId, commitId + "~1"});
  QList<GitFile> list;
  for (auto file : QString::fromLocal8Bit(process.standardOutOutput,
                                          process.standardOutOutput.size())
                       .split('\0', Qt::SkipEmptyParts)) {
    auto gitFile = GitFile();
    gitFile.path = file;
    list.append(gitFile);
  }

  emit historyFilesChanged(commitId, list);
}

void GitInterface::log() {
  emit actionStarted(ActionTag::GIT_LOG);
  WATCH_ASYNC_METHOD_CALL(
      ActionTag::GIT_LOG,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::log));
}

QFuture<void> GitInterface::fetch() {
  RUN_ONCE(ActionTag::GIT_FETCH,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::fetch));
}

QFuture<bool> GitInterface::commit(const QString &message) {
  RUN_ONCE_TYPED(
      ActionTag::GIT_COMMIT, bool,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::commit, message));
}

QFuture<void> GitInterface::stageFile(const QString &path) {
  RUN_ONCE(ActionTag::GIT_ADD,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::stageFiles,
                             (QStringList() << path)));
}

QFuture<void> GitInterface::stageFiles(const QStringList &paths) {
  RUN_ONCE(
      ActionTag::GIT_ADD,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::stageFiles, paths));
}

QFuture<void> GitInterface::unstageFile(const QString &path) {
  RUN_ONCE(
      ActionTag::GIT_RESET,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::unstageFile, path));
}

void GitInterface::selectFile(bool unstaged, const QString &path) {
  emit fileSelected(unstaged, path);
  diffFile(unstaged, path);
}

void GitInterface::diffFile(bool unstaged, const QString &path) {
  emit actionStarted(ActionTag::GIT_DIFF);
  WATCH_ASYNC_METHOD_CALL(ActionTag::GIT_DIFF,
                          QtConcurrent::run(_impl.get(),
                                            &GitInterfacePrivate::diffFile,
                                            unstaged, path, nullptr));
}

void GitInterface::historyDiffFile(const QString &commitId,
                                   const QString &path) {
  emit actionStarted(ActionTag::GIT_DIFF);
  WATCH_ASYNC_METHOD_CALL(ActionTag::GIT_DIFF,
                          QtConcurrent::run(_impl.get(),
                                            &GitInterfacePrivate::diffFile,
                                            false, path, commitId));
}

QFuture<void> GitInterface::addLines(const QList<GitDiffLine> &lines,
                                     bool unstage) {
  RUN_ONCE(unstage ? ActionTag::GIT_RESET : ActionTag::GIT_ADD,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::addLines, lines,
                             unstage));
}

QFuture<void> GitInterface::push(const QString &remote, const QVariant &branch,
                                 bool setUpstream) {
  RUN_ONCE(ActionTag::GIT_PUSH,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::push, remote,
                             branch, setUpstream));
}

QFuture<void> GitInterface::pushTags(const QString &remote) {
  RUN_ONCE(
      ActionTag::GIT_PUSH,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::pushTags, remote));
}

QFuture<void> GitInterface::pull(bool rebase) {
  RUN_ONCE(ActionTag::GIT_PULL,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::pull, rebase));
}

QFuture<void> GitInterface::revertLastCommit() {
  RUN_ONCE(
      ActionTag::GIT_RESET,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::revertLastCommit));
}

QFuture<void> GitInterface::resetLines(const QList<GitDiffLine> &lines) {
  RUN_ONCE(
      ActionTag::GIT_CHECKOUT,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::resetLines, lines));
}

QFuture<void> GitInterface::checkoutPath(const QString &path) {
  RUN_ONCE(
      ActionTag::GIT_CHECKOUT,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::checkoutPath, path));
}

QFuture<void> GitInterface::changeBranch(const QString &branchName,
                                         const QString &upstreamBranchName) {
  RUN_ONCE(ActionTag::GIT_CHECKOUT,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::changeBranch,
                             branchName, upstreamBranchName));
}

QFuture<void> GitInterface::createBranch(const QString &name,
                                         const QString &baseCommit) {
  RUN_ONCE(ActionTag::GIT_BRANCH,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::createBranch,
                             name, baseCommit));
}

QFuture<void> GitInterface::deleteBranch(const QString &name) {
  RUN_ONCE(
      ActionTag::GIT_BRANCH,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::deleteBranch, name));
}

QFuture<void> GitInterface::setUpstream(const QString &remote,
                                        const QString &branch) {
  RUN_ONCE(ActionTag::GIT_REMOTE,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::setUpstream,
                             remote, branch));
}

QFuture<void> GitInterface::createTag(const QString &name,
                                      const QString &commitId) {
  RUN_ONCE(ActionTag::GIT_TAG,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::createTag, name,
                             commitId));
}

QFuture<void> GitInterface::deleteTag(const QString &name) {
  RUN_ONCE(
      ActionTag::GIT_TAG,
      QtConcurrent::run(_impl.get(), &GitInterfacePrivate::deleteTag, name));
}

QFuture<void> GitInterface::stash() {
  RUN_ONCE(ActionTag::GIT_STASH,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::stash));
}

QFuture<void> GitInterface::stashPop() {
  RUN_ONCE(ActionTag::GIT_STASH_APPLY,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::stashPop));
}
QFuture<void> GitInterface::resetToCommit(const QString &commitId,
                                          const ResetType &type) {
  RUN_ONCE(ActionTag::GIT_RESET,
           QtConcurrent::run(_impl.get(), &GitInterfacePrivate::resetToCommit,
                             commitId, type));
}

QFuture<void> GitInterface::cherryPickCommit(const QString &commitId) {
  RUN_ONCE(ActionTag::GIT_CHERRY_PICK,
           QtConcurrent::run(_impl.get(),
                             &GitInterfacePrivate::cherryPickCommit, commitId,
                             QStringList{}));
}
