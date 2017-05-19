#ifndef GITDIFF_H
#define GITDIFF_H

#include <QObject>

class GitDiffLine : public QObject
{
  Q_OBJECT
public:
  enum class diffType
  {
    FILE_HEADER = 0,
    FILE_FOOTER,
    HEADER,
    CONTEXT,
    ADD,
    REMOVE
  };
  Q_ENUM(diffType)

  explicit GitDiffLine(QObject *parent = 0);
  Q_PROPERTY(diffType type MEMBER type NOTIFY typeChanged)
  Q_PROPERTY(QString content MEMBER content NOTIFY contentChanged)
  Q_PROPERTY(int oldLine MEMBER oldLine NOTIFY oldLineChanged)
  Q_PROPERTY(int newLine MEMBER newLine NOTIFY newLineChanged)

  diffType type;
  QString content;
  int oldLine, newLine;

signals:
  void typeChanged();
  void contentChanged();
  void oldLineChanged();
  void newLineChanged();

public slots:
};

#endif // GITDIFF_H
