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

void gitBin::GitManager::openRepository(const QString &path)
{
}

QList<GitFile *> gitBin::GitManager::status()
{
  _impl->process->setArguments({"status", "--porcelain=v1"});
  _impl->process->start(QIODevice::ReadOnly);
  QByteArray output = _impl->process->readLine(1024);

  while (output.length() > 0)
  {
    emit gitError(QString(output));
  }

  return {new GitFile};
}

QList<GitDiffLine *> gitBin::GitManager::diffPath(const QString &path)
{
  return {new GitDiffLine};
}

QString gitBin::GitManager::headName()
{
  _impl->process->setArguments({"branch"});
  _impl->process->start(QIODevice::ReadOnly);

  return QString(_impl->process->readAll());
}

void gitBin::GitManager::stagePath(const QString &path)
{

}

void gitBin::GitManager::unstagePath(const QString &path)
{

}

void gitBin::GitManager::commit(const QString &message)
{

}
