#include "historyfiles.h"
#include "qtreewidgetutils.hpp"
#include "ui_historyfiles.h"

#include "core.hpp"
#include "mainwindow.hpp"
#include "project.hpp"

struct HistoryFilesPrivate {
  HistoryFiles *_this;
  QString currentCommitId;

  HistoryFilesPrivate(HistoryFiles *_this) : _this(_this) {}

  void connectSignals() {
    QObject::connect(
        _this->ui->files, &QTreeWidget::itemClicked, _this,
        [=, this](QTreeWidgetItem *item, int) {
          if (item->data(0, Qt::UserRole).isValid()) {
            _this->core()->project()->activeRepository()->historyDiffFile(
                currentCommitId, item->data(0, Qt::UserRole).toString());
          }
        });
  }
};

DOCK_WIDGET_IMPL(HistoryFiles, "History files")
HistoryFiles::HistoryFiles(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::HistoryFiles),
      _impl(new HistoryFilesPrivate(this)) {
  ui->setupUi(this);
  _impl->connectSignals();
}

HistoryFiles::~HistoryFiles() { delete ui; }

void HistoryFiles::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  ui->commitId->setText(tr("No commit selected"));
  ui->files->clear();

  connect(newGitInterface.get(), &GitInterface::historyFilesChanged,
          activeRepositoryContext.get(),
          [=, this](const QString &commitId, const QList<GitFile> &files) {
            ui->commitId->setText(
                QString(tr("Selected commit id: %1")).arg(commitId));
            _impl->currentCommitId = commitId;
            ui->files->clear();

            QStringList fileNames;
            for (const auto &file : files) {
              fileNames.append(file.path);
            }

            TreeWidgetUtils::createItems(ui->files, fileNames);

            ui->files->expandAll();
          });
}
