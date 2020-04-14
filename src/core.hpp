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

  bool init();

  void changeProject(Project *newProject);
  Project *project() const;

signals:
  void projectChanged(Project *project);

private:
  QScopedPointer<CoreImpl> _impl;
};

#endif // CORE_HPP
