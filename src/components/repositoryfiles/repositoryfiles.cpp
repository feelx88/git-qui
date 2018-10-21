#include "repositoryfiles.hpp"
#include "ui_repositoryfiles.h"

#include <QMainWindow>

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

    auto signal = unstaged ? &GitInterface::nonStagingAreaChanged : &GitInterface::stagingAreaChanged;
    _this->connect(gitInterface.get(), signal, _this, [_this](QList<GitFile> files){
      _this->ui->listWidget->clear();
      for(auto file: files)
      {
        _this->ui->listWidget->addItem(file.path);
      }
    });
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
  return QVariant(config);
}
