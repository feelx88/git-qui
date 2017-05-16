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
    HEADER,
    CONTEXT,
    ADD,
    REMOVE
  };
  Q_ENUM(diffType)

  explicit GitDiffLine(QObject *parent = 0);
  Q_PROPERTY(diffType type MEMBER type NOTIFY typeChanged)
  Q_PROPERTY(QString content MEMBER content NOTIFY contentChanged)

  diffType type;
  QString content;

signals:
  void typeChanged();
  void contentChanged();

public slots:
};

#endif // GITDIFF_H
