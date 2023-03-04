#include "commit.hpp"
#include "ui_commit.h"

#include <QMenu>
#include <QMessageBox>
#include <QShortcut>

#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "project.hpp"

struct CommitPrivate {
  QSharedPointer<GitInterface> gitInterface = nullptr;
  QList<QString> messageHistory;
  QMenu *messageMenu;

  void connectSignals(Commit *_this) {
    _this->connect(
        _this->ui->pushButton, &QPushButton::clicked, _this, [=, this] {
          if (gitInterface->stagedFiles().empty()) {
            QMessageBox dlg;
            dlg.setText(_this->tr("There are no staged files. Would you like "
                                  "to commit everything?"));
            dlg.addButton(QMessageBox::Yes);
            dlg.addButton(QMessageBox::No);
            dlg.setDefaultButton(QMessageBox::Yes);

            if (dlg.exec() == QMessageBox::Yes) {
              QStringList paths;
              for (auto &file : gitInterface->unstagedFiles()) {
                paths << file.path;
              }
              gitInterface->stageFiles(paths).waitForFinished();
            } else {
              return;
            }
          }

          QString message = _this->ui->plainTextEdit->toPlainText();

          if (_this->ui->checkBoxPrefix->isChecked()) {
            message.prepend(_this->ui->lineEditPrefix->text() + " ");
          }

          if (_this->ui->checkBoxSuffix->isChecked()) {
            message.append(QString(" ") + _this->ui->lineEditSuffix->text());
          }

          auto commit = gitInterface->commit(message);
          if (!commit.isCanceled() && commit.result()) {
            emit gitInterface->fileDiffed("", {}, false);
            _this->ui->pushButton_2->setEnabled(true);

            addHistoryEntry(_this, message);
            _this->ui->plainTextEdit->clear();
          }
        });

    QShortcut *shortcut =
        new QShortcut(QKeySequence("Ctrl+Return"), _this->ui->plainTextEdit);
    _this->connect(shortcut, &QShortcut::activated, _this->ui->pushButton,
                   &QPushButton::click);

    _this->ui->pushButton_3->setEnabled(!messageHistory.empty());
    messageMenu = new QMenu(_this);
    _this->ui->pushButton_3->setMenu(messageMenu);
  }

  void addHistoryEntry(Commit *_this, QString message) {
    if (message.isEmpty()) {
      return;
    }

    message.replace("\n", " ");

    messageHistory.removeAll(message);
    messageHistory.push_back(message);

    messageMenu->clear();
    for (const QString &message : qAsConst(messageHistory)) {
      auto truncatedMessage = message;

      if (truncatedMessage.length() > 100) {
        truncatedMessage.truncate(100);
        truncatedMessage.append("...");
      }

      messageMenu->addAction(truncatedMessage, messageMenu, [=, this] {
        _this->ui->plainTextEdit->setPlainText(message);
      });
    }

    _this->ui->pushButton_3->setEnabled(true);
  }
};

DOCK_WIDGET_IMPL(Commit, tr("Commit editor"))

Commit::Commit(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::Commit), _impl(new CommitPrivate) {
  ui->setupUi(this);

  _impl->connectSignals(this);
}

Commit::~Commit() { delete ui; }

QVariant Commit::configuration() {
  return QVariantMap(
      {{"messageHistory", QVariant(_impl->messageHistory.mid(
                              _impl->messageHistory.length() - 20))},
       {"message", QVariant(ui->plainTextEdit->toPlainText())}});
}

void Commit::configure(const QVariant &configuration) {
  QVariantMap config = configuration.toMap();
  auto history = config.value("messageHistory").toStringList();
  for (const QString &message : history) {
    _impl->addHistoryEntry(this, message);
  }
  ui->plainTextEdit->setPlainText(config.value("currentMessage").toString());
}

QVariantMap Commit::getProjectSpecificConfiguration() {
  return QVariantMap{
      {"prependPrefix", ui->checkBoxPrefix->isChecked()},
      {"appendSuffix", ui->checkBoxSuffix->isChecked()},
      {"prefix", ui->lineEditPrefix->text()},
      {"suffix", ui->lineEditSuffix->text()},
      {"message", ui->plainTextEdit->toPlainText()},
  };
}

void Commit::onProjectSwitched(Project *newProject) {
  _impl->gitInterface = nullptr;
  DockWidget::onProjectSwitched(newProject);
}

void Commit::onProjectSpecificConfigurationLoaded(
    const QVariantMap &configuration) {
  ui->checkBoxPrefix->setChecked(
      configuration.value("prependPrefix", false).toBool());
  ui->checkBoxSuffix->setChecked(
      configuration.value("appendSuffix", false).toBool());
  ui->lineEditPrefix->setText(configuration.value("prefix", "").toString());
  ui->lineEditSuffix->setText(configuration.value("suffix", "").toString());
  ui->plainTextEdit->setPlainText(
      configuration.value("message", "").toString());
}

void Commit::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);

  _impl->gitInterface = newGitInterface;

  connect(ui->pushButton_2, &QPushButton::clicked,
          activeRepositoryContext.get(),
          [=, this] { newGitInterface->revertLastCommit(); });

  connect(_impl->gitInterface.get(), &GitInterface::lastCommitReverted,
          activeRepositoryContext.get(), [=, this](const QString &message) {
            ui->plainTextEdit->setPlainText(message);
            ui->pushButton_2->setDisabled(true);
          });
}

void Commit::onError(const QString &message, GitInterface::ActionTag tag,
                     GitInterface::ErrorType) {
  if (tag != GitInterface::ActionTag::GIT_COMMIT) {
    return;
  }

  QString extendedMessage =
      message.isEmpty() ? QObject::tr("There was no error output.")
                        : QObject::tr("The following error output was "
                                      "generated by the commit call: \n") +
                              message;

  QMessageBox dialog(QMessageBox::Warning,
                     QObject::tr("Error on Commit for repository ") +
                         static_cast<GitInterface *>(QObject::sender())->name(),
                     QObject::tr("Could not commit. ") + extendedMessage,
                     QMessageBox::Ok);

  dialog.exec();
}
