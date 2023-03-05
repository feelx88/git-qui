#include "diffview.hpp"
#include "ui_diffview.h"

#include <QAction>
#include <QFontDatabase>
#include <QFutureWatcher>

#include "gitdiffline.hpp"
#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "project.hpp"
#include "treewidgetitem.hpp"

struct DiffViewPrivate {
  DiffView *_this;
  bool historyMode = false;
  QSharedPointer<GitInterface> gitInterface = nullptr;
  bool unstaged = false;
  QString currentPath;
  QAction *fullFileDiffAction, *historyAction, *stageOrUnstageSelected,
      *resetSelected;
  int nonTrivialLines;
  QString hash = "###";

  DiffViewPrivate(DiffView *diffView) : _this(diffView) {}

  void clear() {
    _this->setWindowTitle(_this->tr("Diff view"));
    _this->ui->treeWidget->clear();
    stageOrUnstageSelected->setVisible(false);
    resetSelected->setVisible(false);
  }

  void connectSignals() {
    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked,
                   _this, [=, this] { stageOrUnstage(); });
  }

  void addActions() {
    fullFileDiffAction = new QAction(QObject::tr("Full file diff"), _this);
    fullFileDiffAction->setCheckable(true);
    _this->addAction(fullFileDiffAction);
    _this->connect(fullFileDiffAction, &QAction::toggled, _this,
                   [=, this](bool checked) {
                     gitInterface->setFullFileDiff(checked);
                     gitInterface->diffFile(unstaged, currentPath);
                   });

    historyAction =
        new QAction(QObject::tr("Show only historical diffs"), _this);
    historyAction->setCheckable(true);
    _this->addAction(historyAction);
    _this->connect(historyAction, &QAction::toggled, _this,
                   [=, this](bool checked) { historyMode = checked; });
  }

  void addContextMenuActions() {
    stageOrUnstageSelected = new QAction(_this);
    _this->connect(stageOrUnstageSelected, &QAction::triggered, _this,
                   [=, this] { stageOrUnstage(); });

    resetSelected = new QAction(_this->tr("Reset selected lines"), _this);
    _this->connect(resetSelected, &QAction::triggered, _this, [=, this] {
      QList<GitDiffLine> lines;
      auto selectedItems = _this->ui->treeWidget->selectedItems();
      for (auto item : selectedItems) {
        lines.append(item->data(2, Qt::UserRole).value<GitDiffLine>());
      }
      bool stillUnstaged =
          lines.count() == nonTrivialLines ? !unstaged : unstaged;
      gitInterface->resetLines(lines);
      emit gitInterface->fileSelected(stillUnstaged, currentPath);
    });

    _this->ui->treeWidget->addActions(
        QList<QAction *>() << stageOrUnstageSelected << resetSelected);
  }

  void stageOrUnstage() {
    QList<GitDiffLine> lines;
    auto selectedItems = _this->ui->treeWidget->selectedItems();
    for (auto item : selectedItems) {
      lines.append(item->data(2, Qt::UserRole).value<GitDiffLine>());
    }
    bool stillUnstaged =
        lines.count() == nonTrivialLines ? !unstaged : unstaged;
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(_this);
    watcher->setFuture(gitInterface->addLines(lines, unstaged));
    QObject::connect(
        watcher, &QFutureWatcher<void>::finished, watcher,
        [=, this] { gitInterface->selectFile(stillUnstaged, currentPath); });
  }

  void refreshView(const QString &path, QList<GitDiffLine> lines, bool unstaged,
                   const QString &commitId) {
    if (historyMode) {
      _this->ui->label->setText(QString("%1 @ %2").arg(path).arg(commitId));
    } else {
      _this->ui->label->setText(path);
      stageOrUnstageSelected->setVisible(true);
      resetSelected->setVisible(true);
      resetSelected->setEnabled(unstaged);
    }

    QString newHash = path;
    for (auto &line : lines) {
      newHash.append(line.header).append(line.content);
    }

    if (newHash == hash) {
      return;
    }

    hash = newHash;
    currentPath = path;
    unstaged = unstaged;
    stageOrUnstageSelected->setText(
        unstaged ? QObject::tr("Stage selected lines")
                 : QObject::tr("Unstage selected lines"));
    _this->ui->treeWidget->clear();

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QFontMetrics metrics(font);
    int col0Width = 0, col1Width = 0;
    QTreeWidgetItem *firstInterestingItem = nullptr;
    nonTrivialLines = 0;

    for (const auto &line : lines) {
      if (fullFileDiffAction->isChecked() &&
          line.type == GitDiffLine::diffType::HEADER) {
        continue;
      }

      TreeWidgetItem *item = new TreeWidgetItem(_this->ui->treeWidget);
      item->setFont(2, font);
      item->setData(2, Qt::UserRole, QVariant::fromValue(line));

      QString content = line.content == "\n" ? "" : line.content;

      switch (line.type) {
      case GitDiffLine::diffType::ADD:
        item->setBackground(2, QColor::fromRgb(86, 244, 66));
        item->setForeground(2, Qt::black);
        item->setText(2, "+ " + content);
        firstInterestingItem =
            firstInterestingItem ? firstInterestingItem : item;
        ++nonTrivialLines;
        break;
      case GitDiffLine::diffType::REMOVE:
        item->setBackground(2, QColor::fromRgb(244, 66, 66));
        item->setForeground(2, Qt::black);
        item->setText(2, "- " + content);
        firstInterestingItem =
            firstInterestingItem ? firstInterestingItem : item;
        ++nonTrivialLines;
        break;
      case GitDiffLine::diffType::CONTEXT:
        item->setBackground(2, Qt::white);
        item->setForeground(2, Qt::gray);
        item->setText(2, "  " + content);
        break;
      case GitDiffLine::diffType::FILE_FOOTER:
      case GitDiffLine::diffType::FILE_HEADER:
      case GitDiffLine::diffType::HEADER:
      default:
        item->setForeground(2, Qt::black);
        item->setBackground(2, Qt::gray);
        item->setText(2, content);
        break;
      }

      item->setText(0, line.oldLine > 0 ? QString::number(line.oldLine) : "");
      item->setText(1, line.newLine > 0 ? QString::number(line.newLine) : "");
      item->setBackground(0, Qt::gray);
      item->setBackground(1, Qt::gray);

      col0Width = std::max(
          col0Width,
          metrics.size(Qt::TextSingleLine, QString::number(line.oldLine))
              .width());
      col1Width = std::max(
          col1Width,
          metrics.size(Qt::TextSingleLine, QString::number(line.newLine))
              .width());

      _this->ui->treeWidget->addTopLevelItem(item);
    }

    _this->ui->treeWidget->setColumnWidth(0, col0Width + 10);
    _this->ui->treeWidget->setColumnWidth(1, col1Width + 10);

    _this->ui->treeWidget->scrollToItem(firstInterestingItem);
  }
};

DOCK_WIDGET_IMPL(DiffView, tr("Diff view"))

DiffView::DiffView(MainWindow *mainWindow)
    : DockWidget(mainWindow), ui(new Ui::DiffView),
      _impl(new DiffViewPrivate(this)) {
  ui->setupUi(this);

  _impl->connectSignals();
  _impl->addActions();
}

DiffView::~DiffView() { delete ui; }

QVariant DiffView::configuration() {
  QMap<QString, QVariant> config;
  config.insert("fullFileDiff", _impl->fullFileDiffAction->isChecked());
  config.insert("historyMode", _impl->historyAction->isChecked());

  return config;
}

void DiffView::configure(const QVariant &configuration) {
  auto map = configuration.toMap();
  _impl->fullFileDiffAction->setChecked(
      map.value("fullFileDiff", false).toBool());
  _impl->historyAction->setChecked(map.value("historyMode", false).toBool());

  if (!_impl->historyMode) {
    _impl->addContextMenuActions();
  }
}

void DiffView::onProjectSwitched(Project *newProject) {
  _impl->gitInterface = nullptr;
  DockWidget::onProjectSwitched(newProject);
}

void DiffView::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  DockWidget::onRepositorySwitched(newGitInterface, activeRepositoryContext);
  _impl->clear();
  _impl->gitInterface = newGitInterface;

  connect(
      newGitInterface.get(), &GitInterface::fileDiffed,
      activeRepositoryContext.get(),
      [=, this](const QString &path, QList<GitDiffLine> lines, bool unstaged) {
        if (_impl->historyMode) {
          return;
        }
        _impl->refreshView(path, lines, unstaged, QString());
      });

  connect(newGitInterface.get(), &GitInterface::historyFileDiffed,
          activeRepositoryContext.get(),
          [=, this](const QString &path, QList<GitDiffLine> lines,
                    const QString &commitId) {
            if (!_impl->historyMode) {
              return;
            }
            _impl->refreshView(path, lines, false, commitId);
          });
}
