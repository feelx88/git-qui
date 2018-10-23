#ifndef GITDIFFLINE_H
#define GITDIFFLINE_H

#include <QString>

struct GitDiffLine
{
  enum class diffType
  {
    NONE = 0,
    FILE_HEADER,
    FILE_FOOTER,
    HEADER,
    CONTEXT,
    ADD,
    REMOVE
  };

  diffType type = diffType::NONE;
  QString content = "";
  int oldLine = -1, newLine = -1;
  QString header = "";
  int index = -1;
};

#endif // GITDIFFLINE_H
