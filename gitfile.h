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

  QString path;
  bool modified;
  bool staged;

signals:
  void pathChanged();
  void modifiedChanged();
  void modifiedStaged();
};

#endif // GITFILE_H
