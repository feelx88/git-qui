#include "repositoryfiles.hpp"
#include "ui_repositoryfiles.h"

#include <QAction>
#include <QInputDialog>

#include "mainwindow.hpp"

struct RepositoryFilesPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  bool unstaged;
  QString selection;

  void connectSignals(RepositoryFiles *_this)
  {
    _this->connect(_this->ui->radioButton, &QRadioButton::clicked, _this, [_this]{
      _this->ui->stackedWidget->setCurrentIndex(0);
    });

    _this->connect(_this->ui->radioButton_2, &QRadioButton::clicked, _this, [_this]{
      _this->ui->stackedWidget->setCurrentIndex(1);
    });

    _this->connect(_this->ui->stackedWidget, &QStackedWidget::currentChanged, _this, [=](int index){
      if (index == 1) {
        _this->ui->radioButton_2->setChecked(true);
      }
    });

    auto signal = unstaged ? &GitInterface::nonStagingAreaChanged : &GitInterface::stagingAreaChanged;

    _this->connect(_this->mainWindow(), &MainWindow::repositorySwitched, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      _this->disconnect(gitInterface.get(), signal, _this, nullptr);
      _this->disconnect(gitInterface.get(), &GitInterface::fileSelected, _this, nullptr);

      gitInterface = newGitInterface;

      _this->connect(gitInterface.get(), signal, _this, [=](const QList<GitFile> &files){
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

      _this->connect(gitInterface.get(), &GitInterface::fileSelected, _this, [=](bool unstaged, const QString &path){
        if (unstaged != this->unstaged)
        {
          _this->ui->listWidget->setCurrentItem(nullptr);
          _this->ui->treeWidget->setCurrentItem(nullptr);
          return;
        }

        QList<QListWidgetItem*> listItems = _this->ui->listWidget->findItems(path, Qt::MatchCaseSensitive);
        _this->ui->listWidget->setCurrentItem(listItems.empty() ? nullptr : listItems.first());

        QList<QTreeWidgetItem*> treeItems = _this->ui->treeWidget->findItems(path.split('/').last(), Qt::MatchCaseSensitive | Qt::MatchRecursive);
        _this->ui->treeWidget->setCurrentItem(treeItems.empty() ? nullptr : treeItems.first());
      });
    });

    _this->connect(_this->ui->listWidget, &QListWidget::itemSelectionChanged, _this, [=]{
      auto item = _this->ui->listWidget->currentItem();
      if (item && !item->text().isEmpty())
      {
        gitInterface->selectFile(unstaged, item->text());
      }
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemSelectionChanged, _this, [=]{
      auto item = _this->ui->treeWidget->currentItem();
      if (item && !item->data(0, Qt::UserRole).toString().isEmpty())
      {
        gitInterface->selectFile(unstaged, item->data(0, Qt::UserRole).toString());
      }
    });

    _this->connect(_this->ui->listWidget, &QListWidget::itemDoubleClicked, _this, [=](QListWidgetItem *item){
      stageOrUnstage(_this, item->text());
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, _this, [=](QTreeWidgetItem *item){
      QVariant data = item->data(0, Qt::UserRole);
      if (data.isValid())
      {
        stageOrUnstage(_this, data.toString());
      }
    });
  }

  void addContextMenuActions(RepositoryFiles *_this)
  {
      QList<QAction*> actions;
      QAction *stageOrUnstageAction = new QAction(unstaged ? _this->tr("Stage") : _this->tr("Unstage"), _this);
      _this->connect(stageOrUnstageAction, &QAction::triggered, _this, [=]{
        if (_this->ui->stackedWidget->currentIndex() == 0)
        {
          if (_this->ui->listWidget->currentItem())
          {
            stageOrUnstage(_this, _this->ui->listWidget->currentItem()->text());
          }
        }
        else
        {
          if (_this->ui->treeWidget->currentItem())
          {
            stageOrUnstage(_this, _this->ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString());
          }
        }
      });

      QAction *checkoutAction = new QAction(_this->tr("Reset file"), _this);
      _this->connect(checkoutAction, &QAction::triggered, _this, [=]{
        if (_this->ui->stackedWidget->currentIndex() == 0)
        {
          if (_this->ui->listWidget->currentItem())
          {
            gitInterface->checkoutPath(_this->ui->listWidget->currentItem()->text());
          }
        }
        else
        {
          if (_this->ui->treeWidget->currentItem())
          {
            gitInterface->checkoutPath(_this->ui->treeWidget->currentItem()->data(0, Qt::UserRole).toString());
          }
        }
        emit gitInterface->fileDiffed("", {}, unstaged);
      });

      actions << stageOrUnstageAction;

      if (unstaged) {
        actions << checkoutAction;
      }

      _this->ui->listWidget->addActions(actions);
      _this->ui->treeWidget->addActions(actions);
  }

  void stageOrUnstage(RepositoryFiles *_this, const QString &path)
  {
      if (unstaged)
      {
          gitInterface->stageFile(path);
      }
      else
      {
          gitInterface->unstageFile(path);
      }
      emit gitInterface->fileSelected(!unstaged, path);
      _this->ui->listWidget->setCurrentItem(nullptr);
      _this->ui->treeWidget->setCurrentItem(nullptr);
      selection = "";
  }
};

DOCK_WIDGET_IMPL(
  RepositoryFiles,
  tr("Repository files")
)

RepositoryFiles::RepositoryFiles(MainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface) :
DockWidget(mainWindow),
ui(new Ui::RepositoryFiles),
_impl(new RepositoryFilesPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;
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

void RepositoryFiles::configure(const QVariant &configuration)
{
  auto map = configuration.toMap();

  if (map.empty())
  {
    map.insert("unstaged", QInputDialog::getItem(
      this,
      "Select widget type",
      "Please choose the type of files displayed for this widget.",
      {tr("Unstaged"), tr("Staged")},
      0,
      false
    ) == tr("Unstaged"));
  }

  _impl->unstaged = map.value("unstaged", true).toBool();
  ui->stackedWidget->setCurrentIndex(map.value("selectedViewIndex", 0).toInt());

  _impl->connectSignals(this);
  _impl->addContextMenuActions(this);

  setWindowTitle(_impl->unstaged ? tr("Unstaged files") : tr("Staged files"));
}
