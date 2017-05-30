#ifndef AGITMANAGER_H
#define AGITMANAGER_H

#include <QObject>
#include <QVariant>

class GitDiffLine;
class GitFile;
class GitCommit;

 class AGitManager  : public QObject
{
   Q_OBJECT
 public:
   AGitManager(QObject *parent);
   virtual void init() = 0;
   virtual void openRepository(const QString &path) = 0;
   virtual QString repositoryRoot(const QString &path) = 0;
   virtual QStringList repositoryFiles() = 0;

   virtual QList<GitFile*> status() = 0;
   virtual QList<GitDiffLine*> diffPath(const QString &path, bool diffStaged = false) = 0;
   virtual void stageLines(const QList<GitDiffLine *> &lines, bool revert) = 0;
   virtual QList<GitCommit*> log() = 0;

   virtual Q_INVOKABLE QString headName() = 0;
   virtual Q_INVOKABLE void stagePath(const QString &path) = 0;
   virtual Q_INVOKABLE void unstagePath(const QString &path) = 0;
   virtual Q_INVOKABLE void commit(const QString &message) = 0;
   virtual Q_INVOKABLE void checkout(const QString &path) = 0;
   virtual Q_INVOKABLE void push(const QString& branch, const QString& remote,
                                 const QString& remoteBranch) = 0;

   // QML-only interface
   Q_INVOKABLE QVariantList diffPathVariant(const QString &path, bool diffStaged = false);
   Q_INVOKABLE QVariantList statusVariant();
   Q_INVOKABLE void stageLinesVariant(const QVariantList &lines, bool reverse);
   Q_INVOKABLE QVariantList logVariant();

   // Some simple default functionality
   Q_INVOKABLE bool removeFile(const QString &path);

 signals:
   void gitError(const QString &message);
   void repositoryChanged(const QString path);
};

#endif // AGITMANAGER_H
