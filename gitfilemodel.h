#ifndef GITFILEMODEL_H
#define GITFILEMODEL_H

#include <QAbstractItemModel>
#include <memory>


class GitFileModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit GitFileModel(QObject *parent = 0);
  virtual ~GitFileModel() = default;

signals:

public slots:

  // QAbstractItemModel interface
public:
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
  virtual QModelIndex parent(const QModelIndex &child) const override;
  virtual int rowCount(const QModelIndex &parent) const override;
  virtual int columnCount(const QModelIndex &) const override;
  virtual QVariant data(const QModelIndex &index, int role) const override;

private:
  struct GitFileModelPrivate;
  std::shared_ptr<GitFileModelPrivate> _impl;
};

#endif // GITFILEMODEL_H
