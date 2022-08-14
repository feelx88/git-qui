#include "errorlog.hpp"
#include "ui_errorlog.h"

#include <QStandardPaths>

struct ErrorLogPrivate {
  QSharedPointer<QFile> getLogFile() {
    auto fileName =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
        "-error.log";
    return QFile::exists(fileName) ? QSharedPointer<QFile>(new QFile(fileName))
                                   : nullptr;
  }
};

DOCK_WIDGET_IMPL(ErrorLog, tr("Error log"))

ErrorLog::ErrorLog(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::ErrorLog), _impl(new ErrorLogPrivate) {
  ui->setupUi(this);

  connect(ui->pushButton, &QPushButton::clicked, this,
          &ErrorLog::onPushButtonClicked);
}

ErrorLog::~ErrorLog() { delete ui; }

void ErrorLog::configure(const QVariant &) {
  auto file = _impl->getLogFile();

  if (file) {
    file->open(QFile::ReadOnly);
    this->ui->plainTextEdit->appendPlainText(file->readAll());
  }
}

void ErrorLog::onError(const QString &message, GitInterface::ActionTag,
                       GitInterface::ErrorType type) {
  if (type == GitInterface::ErrorType::STDERR) {
    this->ui->plainTextEdit->moveCursor(QTextCursor::End);
    this->ui->plainTextEdit->insertPlainText(message);
    this->ui->plainTextEdit->moveCursor(QTextCursor::End);
  }
}

void ErrorLog::onPushButtonClicked() {
  auto file = _impl->getLogFile();

  if (file) {
    file->open(QFile::WriteOnly);
    file->write("");
  }

  this->ui->plainTextEdit->setPlainText("");
}
