#include "gitfilemodel.h"

#include <QDir>

struct GitFileModel::GitFileModelPrivate
{
  ~GitFileModelPrivate() = default;
};

GitFileModel::GitFileModel(QObject *parent)
  : QAbstractItemModel(parent),
    _impl(new GitFileModelPrivate)
{
}

QModelIndex GitFileModel::index(int row, int column, const QModelIndex &parent) const
{
}

QModelIndex GitFileModel::parent(const QModelIndex &child) const
{
}

int GitFileModel::rowCount(const QModelIndex &parent) const
{
}

int GitFileModel::columnCount(const QModelIndex &) const
{
  return 1;
}

QVariant GitFileModel::data(const QModelIndex &index, int role) const
{
}
