#include "logview.hpp"
#include "ui_logview.h"

#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "gitcommit.hpp"
#include "treewidgetitem.hpp"

struct LogViewPrivate
{
  GitInterface *gitInterface = nullptr;

  void connectSignals(LogView *_this)
  {
    _this->ui->treeWidget->setHeaderLabels({
      _this->tr("Id"),
      _this->tr("Branches"),
      _this->tr("Message"),
      _this->tr("Author"),
      _this->tr("Date"),
    });
  }
};

DOCK_WIDGET_IMPL(
    LogView,
    tr("Log view")
)

LogView::LogView(MainWindow *mainWindow) :
DockWidget(mainWindow),
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

void LogView::configure(const QVariant &configuration)
{
  QList<QVariant> widths = configuration.toList();
  for (int x = 0; x < widths.size(); ++x)
  {
    ui->treeWidget->setColumnWidth(x, widths.at(x).toInt());
  }
}

void LogView::onProjectSwitched(Project *newProject)
{
  _impl->gitInterface = nullptr;
  DockWidget::onProjectSwitched(newProject);
}

void LogView::onRepositorySwitched(GitInterface *newGitInterface)
{
  disconnect(_impl->gitInterface, &GitInterface::logChanged, this, nullptr);

  _impl->gitInterface = newGitInterface;

  connect(_impl->gitInterface, &GitInterface::logChanged, this, [=](const QList<GitCommit> &commits){
    ui->treeWidget->clear();
    for (GitCommit commit : commits)
    {
      TreeWidgetItem *item = new TreeWidgetItem(ui->treeWidget);
      item->setText(0, commit.id);
      item->setText(1, commit.branches.join(", "));
      item->setText(2, commit.message);
      item->setText(3, commit.author);
      item->setText(4, commit.date.toString());

      ui->treeWidget->addTopLevelItem(item);
    }

    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(2);
    ui->treeWidget->resizeColumnToContents(3);
  });
}
