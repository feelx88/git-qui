#include "commit.hpp"
#include "ui_commit.h"

#include <QMainWindow>

#include "gitinterface.hpp"

struct CommitPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  void connectSignals(Commit *_this)
  {
    _this->connect(_this->ui->pushButton, &QPushButton::clicked, _this, [=]{
      gitInterface->commit(_this->ui->plainTextEdit->toPlainText());
      _this->ui->plainTextEdit->clear();
    });
  }

  static void initialize(QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new Commit(mainWindow, gitInterface));
  }

  static void restore(QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &)
  {
    Commit *commit = new Commit(mainWindow, gitInterface);
    commit->setObjectName(id);
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, commit);
  }
};

DOCK_WIDGET_IMPL(
  Commit,
  tr("Commit editor"),
  &CommitPrivate::initialize,
  &CommitPrivate::restore
)

Commit::Commit(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
DockWidget(parent),
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
