#ifndef GITFILE_H
#define GITFILE_H

#include <QObject>
#include <memory>

struct GitFile : public QObject
{
  Q_OBJECT
public:
  explicit GitFile(QObject *parent = 0);
  Q_PROPERTY(QString path MEMBER path NOTIFY pathChanged)
  Q_PROPERTY(bool modified MEMBER modified NOTIFY modifiedChanged)
  Q_PROPERTY(bool staged MEMBER staged NOTIFY modifiedStaged)
  Q_PROPERTY(bool unstaged MEMBER unstaged NOTIFY modifiedUnstaged)
  Q_PROPERTY(bool deleted MEMBER deleted NOTIFY modifiedDeleted)

  QString path = "";
  bool modified = false;
  bool unstaged = false;
  bool staged = false;
  bool deleted = false;

signals:
  void pathChanged();
  void modifiedChanged();
  void modifiedStaged();
  void modifiedUnstaged();
  void modifiedDeleted();
};

#endif // GITFILE_H
