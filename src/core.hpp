#ifndef CORE_HPP
#define CORE_HPP

#include <QObject>

struct CoreImpl;
class Project;
class GitInterface;

class Core : public QObject
{
  Q_OBJECT
public:
  friend struct CoreImpl;
  explicit Core(QObject *parent = nullptr);
  virtual ~Core();

  void changeProject(const QString &path);
  Project *project() const;

signals:
  void projectChanged(Project *project);

private:
  QScopedPointer<CoreImpl> _impl;
};

#endif // CORE_HPP
