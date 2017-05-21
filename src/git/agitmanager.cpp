#include <git/agitmanager.h>

#include <QList>
#include <QVariant>

#include <git/gitdiffline.h>
#include <git/gitfile.h>

AGitManager::AGitManager(QObject *parent)
  : QObject (parent)
{
}

QVariantList AGitManager::diffPathVariant(const QString &path)
{
  auto list = diffPath(path);
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
