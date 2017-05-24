#include <git/agitmanager.h>

#include <QList>
#include <QVariant>

#include <git/gitdiffline.h>
#include <git/gitfile.h>

AGitManager::AGitManager(QObject *parent)
  : QObject (parent)
{
}

QVariantList AGitManager::diffPathVariant(const QString &path, bool diffStaged)
{
  auto list = diffPath(path, diffStaged);
  QVariantList propList;

  for(auto *diffLine : list)
  {
    propList.append(qVariantFromValue(diffLine));
  }

  return propList;
}

QVariantList AGitManager::statusVariant()
{
  auto list = status();
  QVariantList propList;

  for(auto *file : list)
  {
    propList.append(qVariantFromValue(file));
  }

  return propList;
}

void AGitManager::stageLinesVariant(const QVariantList &lines, bool reverse)
{
  QList<GitDiffLine*> list;

  for (auto line : lines) {
    QVariantMap obj = line.toMap();
    GitDiffLine *diffLine = new GitDiffLine(this);

    diffLine->header = obj.value("header").toString();
    diffLine->content = obj.value("content").toString();
    diffLine->oldLine = obj.value("oldLine").toInt();
    diffLine->newLine = obj.value("newLine").toInt();
    diffLine->type = static_cast<GitDiffLine::diffType>(obj.value("type").toInt());

    list.append(diffLine);
  }

  stageLines(list, reverse);
}
