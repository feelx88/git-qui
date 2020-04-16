#ifndef CORE_HPP
#define CORE_HPP

#include <QObject>

struct CorePrivate;
class Project;
class GitInterface;

class Core : public QObject
{
  Q_OBJECT
public:
  friend struct CorePrivate;
  explicit Core(QObject *parent = nullptr);
  virtual ~Core();

  bool init();

  void changeProject(Project *newProject);
  Project *project() const;

  QVariantMap recentProjects() const;

public slots:
  void clearRecentProjects();

signals:
  void projectChanged(Project *project);

private:
  QScopedPointer<CorePrivate> _impl;
};

#endif // CORE_HPP
