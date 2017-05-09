#ifndef GITFILE_H
#define GITFILE_H

#include <QObject>
#include <memory>

class GitFile : public QObject
{
  Q_OBJECT
public:
  explicit GitFile(QObject *parent = 0);
  Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
  Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

  void setPath(QString path);
  QString path();

  void setModified(bool modified);
  bool modified();

signals:
  void pathChanged();
  void modifiedChanged();

public slots:

private:
  struct GitFilePrivate;
  std::shared_ptr<GitFilePrivate> _impl;
};

#endif // GITFILE_H
