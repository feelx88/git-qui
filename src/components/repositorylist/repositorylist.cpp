#include "repositorylist.h"
#include "ui_repositorylist.h"

#include "mainwindow.hpp"

struct RepositoryListPrivate
{
  QString currentRepository;

  void connectSignals(RepositoryList *_this)
  {
    MainWindow *mainWindow = static_cast<MainWindow*>(_this->parent());

    _this->connect(mainWindow, &MainWindow::repositoryAdded, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      QString directory = newGitInterface->path().split('/').last();
      auto items = _this->ui->treeWidget->findItems(directory, Qt::MatchCaseSensitive, 0);

      if (!items.isEmpty())
      {
        return;
      }

      QTreeWidgetItem *item = new QTreeWidgetItem;
      item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
      item->setText(0, directory);
      item->setData(0, Qt::UserRole, newGitInterface->path());
      item->setTextAlignment(1, Qt::AlignRight);
      _this->ui->treeWidget->addTopLevelItem(item);

      _this->connect(newGitInterface.get(), &GitInterface::branchChanged, _this, [=](const QString &branch, bool hasChanges, bool hasUpstream, int commitsAhead, int commitsBehind){
        if (hasUpstream)
        {
          item->setText(1, QString("%1%2 %3↑ %4↓").arg(branch).arg(hasChanges ? "*" : "").arg(commitsAhead).arg(commitsBehind));
        }
        else
        {
          item->setText(1, QString("%1%2 ∅").arg(branch).arg(hasChanges ? "*" : ""));
        }
        _this->ui->treeWidget->resizeColumnToContents(1);
      });
    });

    _this->connect(mainWindow, &MainWindow::repositoryRemoved, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      auto items = _this->ui->treeWidget->findItems(newGitInterface->path().split('/').last(), Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        delete items.first();
      }

      _this->disconnect(newGitInterface.get(), &GitInterface::branchChanged, _this, nullptr);
    });

    _this->connect(mainWindow, &MainWindow::repositorySwitched, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      currentRepository = newGitInterface->path().split('/').last();
      auto items = _this->ui->treeWidget->findItems(currentRepository, Qt::MatchCaseSensitive, 0);
      if (!items.isEmpty())
      {
        _this->ui->treeWidget->setCurrentItem(items.first());
      }
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemSelectionChanged, _this, [=]{
      if (_this->ui->treeWidget->currentItem())
      {
        mainWindow->switchRepository(_this->ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString());
      }
    });

    _this->connect(_this->ui->pushButton, &QPushButton::clicked, static_cast<MainWindow*>(_this->parent()), &MainWindow::openRepository);
    _this->connect(_this->ui->pushButton_2, &QPushButton::clicked, static_cast<MainWindow*>(_this->parent()), &MainWindow::closeCurrentRepository);
  }

  static void initialize(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    DockWidget::initialize(mainWindow, new RepositoryList(mainWindow, gitInterface));
  }

  static void restore(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &)
  {
    DockWidget::restore(mainWindow, id, new RepositoryList(mainWindow, gitInterface));
  }
};

DOCK_WIDGET_IMPL(
    RepositoryList,
    tr("Repository list"),
    &RepositoryListPrivate::initialize,
    &RepositoryListPrivate::restore
)

RepositoryList::RepositoryList(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
  DockWidget(parent),
  ui(new Ui::RepositoryList),
  _impl(new RepositoryListPrivate)
{
  ui->setupUi(this);

  _impl->connectSignals(this);

  ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

RepositoryList::~RepositoryList()
{
  delete ui;
}
