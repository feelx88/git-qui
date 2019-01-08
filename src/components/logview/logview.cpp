#include "logview.hpp"
#include "ui_logview.h"

#include "mainwindow.hpp"

#include "gitinterface.hpp"
#include "gitcommit.hpp"

struct LogViewPrivate
{
  void connectSignals(LogView *_this)
  {
    _this->ui->treeWidget->setHeaderLabels({
      _this->tr("Id"),
      _this->tr("Message"),
      _this->tr("Author"),
      _this->tr("Date"),
    });

    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositorySwitched, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      _this->connect(newGitInterface.get(), &GitInterface::logChanged, _this, [=](const QList<GitCommit> &commits){
        _this->ui->treeWidget->clear();
        for (GitCommit commit : commits)
        {
          QTreeWidgetItem *item = new QTreeWidgetItem(_this->ui->treeWidget);
          item->setText(0, commit.id);
          item->setText(1, commit.branches.empty() ? commit.message : QString("[%1] %2").arg(commit.branches.join(", "), commit.message));
          item->setText(2, commit.author);
          item->setText(3, commit.date.toString());

          _this->ui->treeWidget->addTopLevelItem(item);
        }

        _this->ui->treeWidget->resizeColumnToContents(0);
        _this->ui->treeWidget->resizeColumnToContents(2);
        _this->ui->treeWidget->resizeColumnToContents(3);
      });
    });
  }

  static void initialize(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new LogView(mainWindow, gitInterface));
  }

  static void restore(QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &configuration)
  {
    LogView *logView = new LogView(mainWindow, gitInterface);
    logView->setObjectName(id);
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, logView);

    QList<QVariant> widths = configuration.toList();
    for (int x = 0; x < widths.size(); ++x)
    {
      logView->ui->treeWidget->setColumnWidth(x, widths.at(x).toInt());
    }
  }
};

DOCK_WIDGET_IMPL(
    LogView,
    tr("Log view"),
    &LogViewPrivate::initialize,
    &LogViewPrivate::restore
)

LogView::LogView(QWidget *parent, QSharedPointer<GitInterface> gitInterface) :
DockWidget(parent),
ui(new Ui::LogView),
_impl(new LogViewPrivate)
{
  ui->setupUi(this);

  _impl->connectSignals(this);
}

LogView::~LogView()
{
  delete ui;
}

QVariant LogView::configuration()
{
  QList<QVariant> widths;

  for (int x = 0; x < ui->treeWidget->columnCount(); ++x)
  {
    widths << ui->treeWidget->columnWidth(x);
  }

  return widths;
}
