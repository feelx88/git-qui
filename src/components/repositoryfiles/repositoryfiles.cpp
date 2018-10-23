#include "repositoryfiles.hpp"
#include "ui_repositoryfiles.h"

#include <QMainWindow>
#include <QAction>

#include "repositoryfilesconfig.hpp"

struct RepositoryFilesPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  bool unstaged;

  void connectSignals(RepositoryFiles *_this)
  {
    _this->connect(_this->ui->radioButton, &QRadioButton::clicked, _this, [_this]{
      _this->ui->stackedWidget->setCurrentIndex(0);
    });

    _this->connect(_this->ui->radioButton_2, &QRadioButton::clicked, _this, [_this]{
      _this->ui->stackedWidget->setCurrentIndex(1);
    });

    _this->connect(_this->ui->stackedWidget, &QStackedWidget::currentChanged, [=](int index){
      if (index == 1) {
        _this->ui->radioButton_2->setChecked(true);
      }
    });

    auto signal = unstaged ? &GitInterface::nonStagingAreaChanged : &GitInterface::stagingAreaChanged;
    _this->connect(gitInterface.get(), signal, _this, [_this](QList<GitFile> files){
      _this->ui->listWidget->clear();
      _this->ui->treeWidget->clear();
      for(auto file: files)
      {
        _this->ui->listWidget->addItem(file.path);

        QList<QString> parts = file.path.split('/');

        int index = parts.size() - 1;
        QTreeWidgetItem *topLevelItem = nullptr;
        for (; index >= 0; --index)
        {
          QList<QTreeWidgetItem*> result = _this->ui->treeWidget->findItems(parts.at(index), Qt::MatchCaseSensitive | Qt::MatchRecursive);
          if (!result.empty())
          {
            topLevelItem = result.first();
            index++;
            break;
          }
        }

        if (!topLevelItem)
        {
          topLevelItem = new QTreeWidgetItem(_this->ui->treeWidget);
          topLevelItem->setText(0, parts.at(0));
          topLevelItem->setData(0, Qt::UserRole, parts.at(0));
          index = 1;
          _this->ui->treeWidget->addTopLevelItem(topLevelItem);
        }

        for (; index < parts.size(); ++index)
        {
          QTreeWidgetItem *child = new QTreeWidgetItem(topLevelItem);
          child->setText(0, parts.at(index));

          child->setData(0, Qt::UserRole, index == parts.size() ? parts.at(index) : parts.mid(0, index + 1).join('/'));

          topLevelItem->addChild(child);
          topLevelItem = child;
        }
      }

      _this->ui->treeWidget->expandAll();
    });

    _this->connect(_this->ui->listWidget, &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item){
      stageOrUnstage(item->text());
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, [=](QTreeWidgetItem *item){
      QVariant data = item->data(0, Qt::UserRole);
      if (data.isValid())
      {
        stageOrUnstage(data.toString());
      }
    });
  }

  void addContextMenuActions(RepositoryFiles *_this)
  {
      QList<QAction*> actions;
      QAction *stageOrUnstageAction = new QAction(unstaged ? _this->tr("Stage") : _this->tr("Unstage"));
      _this->connect(stageOrUnstageAction, &QAction::triggered, [=]{
        if (_this->ui->stackedWidget->currentIndex() == 0)
        {
          stageOrUnstage(_this->ui->listWidget->currentItem()->text());
        }
        else
        {
          stageOrUnstage(_this->ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString());
        }
      });

      actions << stageOrUnstageAction;
      _this->ui->listWidget->addActions(actions);
      _this->ui->treeWidget->addActions(actions);
  }

  void stageOrUnstage(const QString &path)
  {
      if (unstaged)
      {
          gitInterface->stageFile(path);
      }
      else
      {
          gitInterface->unstageFile(path);
      }
  }

  static void initialize(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    RepositoryFilesConfig config;
    config.exec();

    if (config.result() == QDialog::Accepted)
    {
      mainWindow->addDockWidget(Qt::TopDockWidgetArea, new RepositoryFiles(mainWindow, gitInterface, config.unstaged()));
    }
  }

  static void restore(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &configuration)
  {
    QMap<QString, QVariant> config = configuration.toMap();
    RepositoryFiles *repositoryFiles = new RepositoryFiles(mainWindow, gitInterface, config.value("unstaged").toBool());
    repositoryFiles->setObjectName(id);
    repositoryFiles->ui->stackedWidget->setCurrentIndex(config.value("selectedViewIndex", 0).toInt());
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, repositoryFiles);
  }
};

DOCK_WIDGET_IMPL(
  RepositoryFiles,
  tr("Repository files"),
  &RepositoryFilesPrivate::initialize,
  &RepositoryFilesPrivate::restore
)

RepositoryFiles::RepositoryFiles(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface, bool unstaged) :
DockWidget(parent),
ui(new Ui::RepositoryFiles),
_impl(new RepositoryFilesPrivate)
{
  ui->setupUi(this);

  _impl->unstaged = unstaged;

  _impl->gitInterface = gitInterface;
  _impl->connectSignals(this);
  _impl->addContextMenuActions(this);

  setWindowTitle(unstaged ? tr("Unstaged files") : tr("Staged files"));
}

RepositoryFiles::~RepositoryFiles()
{
  delete ui;
}

QVariant RepositoryFiles::configuration()
{
  QMap<QString, QVariant> config;
  config.insert("unstaged", QVariant(_impl->unstaged));
  config.insert("selectedViewIndex", QVariant(ui->stackedWidget->currentIndex()));
  return QVariant(config);
}
