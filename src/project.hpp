#ifndef PROJECT_HPP
#define PROJECT_HPP

struct ProjectImpl;

class Project
{
public:
  Project();

private:
  ProjectImpl *_impl;
};

#endif // PROJECT_HPP
