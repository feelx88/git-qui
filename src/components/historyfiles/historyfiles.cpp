#include "historyfiles.h"
#include "qtreewidgetutils.hpp"
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

            QStringList fileNames;
            for (const auto &file : files) {
              fileNames.append(file.path);
            }

            TreeWidgetUtils::createItems(ui->files, fileNames);

            ui->files->expandAll();
          });
}
