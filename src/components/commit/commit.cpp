#include "commit.hpp"
#include "ui_commit.h"

#include <QMainWindow>

#include "gitinterface.hpp"

struct CommitPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  void connectSignals(Commit *_this)
  {
    _this->connect(_this->ui->pushButton, &QPushButton::clicked, [=]{
      gitInterface->commit(_this->ui->plainTextEdit->toPlainText());
      _this->ui->plainTextEdit->clear();
    });
  }

  static void initialize(QMainWindow *mainWindow, QSharedPointer<GitInterface> gitInterface)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new Commit(mainWindow, gitInterface));
  }
};

DOCK_WIDGET_IMPL(Commit)
{
  return new RegistryEntry {
    tr("Commit editor"),
    &CommitPrivate::initialize
  };
}

Commit::Commit(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
QDockWidget(parent),
ui(new Ui::Commit),
_impl(new CommitPrivate)
{
  ui->setupUi(this);
  _impl->gitInterface = gitInterface;

  _impl->connectSignals(this);
}

Commit::~Commit()
{
  delete ui;
}
