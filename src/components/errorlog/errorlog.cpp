#include "errorlog.hpp"
#include "ui_errorlog.h"

#include <QStandardPaths>

DOCK_WIDGET_IMPL(
    ErrorLog,
    tr("Error log")
)

ErrorLog::ErrorLog(MainWindow *mainWindow)
  : DockWidget(mainWindow),
    ui(new Ui::ErrorLog)
{
  ui->setupUi(this);
}

ErrorLog::~ErrorLog()
{
  delete ui;
}

void ErrorLog::configure(const QVariant &)
{
  auto fileName = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "-error.log";

  if (QFile::exists(fileName))
  {
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    this->ui->plainTextEdit->appendPlainText(file.readAll());
  }
}

void ErrorLog::onError(const QString &message, ErrorTag tag)
{
  if (tag == ErrorTag::STDERR)
  {
    this->ui->plainTextEdit->moveCursor(QTextCursor::End);
    this->ui->plainTextEdit->insertPlainText(message);
    this->ui->plainTextEdit->moveCursor(QTextCursor::End);
  }
}
