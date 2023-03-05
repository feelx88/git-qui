#include "historyfiles.h"
#include "ui_historyfiles.h"

#include "mainwindow.hpp"

DOCK_WIDGET_IMPL(HistoryFiles, "History files")
HistoryFiles::HistoryFiles(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::HistoryFiles) {
  ui->setupUi(this);
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
            ui->files->clear();

            for (const auto &file : files) {
              auto parentItem = ui->files->invisibleRootItem();

              for (const auto &part : file.path.split('/')) {
                QTreeWidgetItem *currentItem = nullptr;

                for (int x = 0; x < parentItem->childCount(); ++x) {
                  if (parentItem->child(x)->text(0) == part) {
                    currentItem = parentItem->child(x);
                    break;
                  }
                }

                if (!currentItem) {
                  currentItem = new QTreeWidgetItem(parentItem, {part});
                }

                parentItem = currentItem;
              }
            }

            ui->files->expandAll();
          });
}
