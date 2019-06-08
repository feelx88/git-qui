#include "logview.hpp"
#include "ui_logview.h"

#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "gitcommit.hpp"

struct LogViewPrivate
{
  GitInterface *gitInterface;

  void connectSignals(LogView *_this)
  {
    _this->ui->treeWidget->setHeaderLabels({
      _this->tr("Id"),
      _this->tr("Branches"),
      _this->tr("Message"),
      _this->tr("Author"),
      _this->tr("Date"),
    });

    _this->connect(_this->mainWindow(), &MainWindow::repositorySwitched, _this, [=](GitInterface *newGitInterface){
      _this->disconnect(gitInterface, &GitInterface::logChanged, _this, nullptr);

      gitInterface = newGitInterface;

      _this->connect(gitInterface, &GitInterface::logChanged, _this, [=](const QList<GitCommit> &commits){
        _this->ui->treeWidget->clear();
        for (GitCommit commit : commits)
        {
          QTreeWidgetItem *item = new QTreeWidgetItem(_this->ui->treeWidget);
          item->setText(0, commit.id);
          item->setText(1, commit.branches.join(", "));
          item->setText(2, commit.message);
          item->setText(3, commit.author);
          item->setText(4, commit.date.toString());

          _this->ui->treeWidget->addTopLevelItem(item);
        }

        _this->ui->treeWidget->resizeColumnToContents(0);
        _this->ui->treeWidget->resizeColumnToContents(2);
        _this->ui->treeWidget->resizeColumnToContents(3);
      });
    });
  }
};

DOCK_WIDGET_IMPL(
    LogView,
    tr("Log view")
)

LogView::LogView(MainWindow *mainWindow, GitInterface *gitInterface) :
DockWidget(mainWindow),
ui(new Ui::LogView),
_impl(new LogViewPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;
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

void LogView::configure(const QVariant &configuration)
{
  QList<QVariant> widths = configuration.toList();
  for (int x = 0; x < widths.size(); ++x)
  {
    ui->treeWidget->setColumnWidth(x, widths.at(x).toInt());
  }
}
