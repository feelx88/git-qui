#include "repositoryfiles.hpp"
#include "ui_repositoryfiles.h"

struct RepositoryFilesPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  void connectSignals(RepositoryFiles *_this)
  {
    _this->connect(_this->ui->radioButton, &QRadioButton::clicked, [_this]{
      _this->ui->stackedWidget->setCurrentIndex(0);
    });

    _this->connect(_this->ui->radioButton_2, &QRadioButton::clicked, [_this]{
      _this->ui->stackedWidget->setCurrentIndex(1);
    });

    _this->connect(gitInterface.get(), &GitInterface::nonStagingAreaChanged, []{});
  }
};

RepositoryFiles::RepositoryFiles(QWidget *parent, QSharedPointer<GitInterface> gitInterface) :
QDockWidget(parent),
ui(new Ui::RepositoryFiles),
_impl(new RepositoryFilesPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;
  _impl->connectSignals(this);
}

RepositoryFiles::~RepositoryFiles()
{
  delete ui;
}
