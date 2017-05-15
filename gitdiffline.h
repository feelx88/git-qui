#ifndef GITDIFF_H
#define GITDIFF_H

#include <QObject>

class GitDiffLine : public QObject
{
  Q_OBJECT
public:
  enum class diffType
  {
    ADD = 0,
    REMOVE
  };
  Q_ENUM(diffType)

  explicit GitDiffLine(QObject *parent = 0);
  Q_PROPERTY(diffType type MEMBER type NOTIFY typeChanged)

  diffType type;

signals:
  void typeChanged();

public slots:
};

#endif // GITDIFF_H
