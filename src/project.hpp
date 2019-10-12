#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QList>

struct ProjectImpl;

class Project
{
public:
  Project();

private:
  ProjectImpl *_impl;
};

#endif // PROJECT_HPP
