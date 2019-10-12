#include "project.hpp"

#include "repository.hpp"

struct ProjectImpl
{
  QList<Repository> repositories;
};

Project::Project()
  : _impl(new ProjectImpl)
{
}
