#include "gitdiffline.hpp"

#include <QDebug>

QDebug operator<<(QDebug dbg, const GitDiffLine &diffLine)
{
  return dbg << static_cast<int>(diffLine.type) << diffLine.content;
}
