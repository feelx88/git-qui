#include "diffview.hpp"
#include "ui_diffview.h"

#include <QFontDatabase>
#include <QAction>

#include "core.hpp"
#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "gitdiffline.hpp"
#include "treewidgetitem.hpp"

struct DiffViewPrivate
{
  DiffView *_this;
  GitInterface *gitInterface = nullptr;
  bool unstaged = false;
  QString currentPath;
  QAction *fullFileDiffAction, *stageOrUnstageSelected, *resetSelected;
  int nonTrivialLines;
  QString hash;

  DiffViewPrivate(DiffView *diffView)
    : _this(diffView)
  {}

  void clear()
  {
    _this->setWindowTitle(_this->tr("Diff view"));
    _this->ui->treeWidget->clear();
    stageOrUnstageSelected->setVisible(false);
    resetSelected->setVisible(false);
  }

  void connectSignals()
  {
    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, _this, [=]{
      stageOrUnstage();
    });
  }

  void addActions()
  {
    fullFileDiffAction = new QAction(_this->tr("Full file diff"), _this);
    fullFileDiffAction->setCheckable(true);
    _this->addAction(fullFileDiffAction);
    _this->connect(fullFileDiffAction, &QAction::toggled, _this, [=](bool checked){
      gitInterface->setFullFileDiff(checked);
      gitInterface->diffFile(unstaged, currentPath);
    });
  }

  void addContextMenuActions()
  {
    stageOrUnstageSelected = new QAction(_this);
    _this->connect(stageOrUnstageSelected, &QAction::triggered, _this, [=]{
      stageOrUnstage();
    });

    resetSelected = new QAction(_this->tr("Reset selected lines"), _this);
    _this->connect(resetSelected, &QAction::triggered, _this, [=]{
      QList<GitDiffLine> lines;
      for (auto item : _this->ui->treeWidget->selectedItems())
      {
        lines.append(item->data(2, Qt::UserRole).value<GitDiffLine>());
      }
      bool stillUnstaged = lines.count() == nonTrivialLines ? !unstaged : unstaged;
      gitInterface->resetLines(lines);
      emit gitInterface->fileSelected(stillUnstaged, currentPath);
    });

    _this->ui->treeWidget->addActions(QList<QAction*>() << stageOrUnstageSelected << resetSelected);
  }

  void stageOrUnstage()
  {
    QList<GitDiffLine> lines;
    for (auto item : _this->ui->treeWidget->selectedItems())
    {
      lines.append(item->data(2, Qt::UserRole).value<GitDiffLine>());
    }
    bool stillUnstaged = lines.count() == nonTrivialLines ? !unstaged : unstaged;
    gitInterface->addLines(lines, unstaged);

    emit gitInterface->fileSelected(stillUnstaged, currentPath);
  }
};

DOCK_WIDGET_IMPL(
    DiffView,
    tr("Diff view")
)

DiffView::DiffView(MainWindow *mainWindow) :
  DockWidget(mainWindow),
  ui(new Ui::DiffView),
  _impl(new DiffViewPrivate(this))
{
  ui->setupUi(this);

  _impl->connectSignals();
  _impl->addActions();
  _impl->addContextMenuActions();
}

DiffView::~DiffView()
{
  delete ui;
}

QVariant DiffView::configuration()
{
  QMap<QString, QVariant> config;
  config.insert("fullFileDiff", _impl->fullFileDiffAction->isChecked());

  return config;
}

void DiffView::configure(const QVariant &configuration)
{
  auto map = configuration.toMap();
  _impl->fullFileDiffAction->setChecked(map.value("fullFileDiff", false).toBool());
}

void DiffView::onProjectSwitched(Project *newProject)
{
  DockWidget::onProjectSwitched(newProject);
  _impl->gitInterface = nullptr;
}

void DiffView::onRepositorySwitched(GitInterface *newGitInterface)
{
  _impl->clear();
  disconnect(_impl->gitInterface, &GitInterface::fileDiffed, this, nullptr);

  _impl->gitInterface = newGitInterface;

  connect(_impl->gitInterface, &GitInterface::fileDiffed, this, [=](const QString &path, QList<GitDiffLine> lines, bool unstaged){
    _impl->stageOrUnstageSelected->setVisible(true);
    _impl->resetSelected->setVisible(true);
    _impl->resetSelected->setEnabled(unstaged);

    QString hash = path;
    for (auto &line : lines)
    {
      hash.append(line.header).append(line.content);
    }

    if (hash == _impl->hash)
    {
      return;
    }

    _impl->hash = hash;
    _impl->currentPath = path;
    _impl->unstaged = unstaged;
    setWindowTitle(path);
    _impl->stageOrUnstageSelected->setText(unstaged ?
      tr("Stage selected lines") : tr("Unstage selected lines"));
    ui->treeWidget->clear();

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QFontMetrics metrics(font);
    int col0Width = 0, col1Width = 0;
    QTreeWidgetItem *firstInterestingItem = nullptr;
    _impl->nonTrivialLines = 0;

    for (auto line : lines)
    {
      if (_impl->fullFileDiffAction->isChecked() && line.type == GitDiffLine::diffType::HEADER)
      {
        continue;
      }

      TreeWidgetItem *item = new TreeWidgetItem(ui->treeWidget);
      item->setFont(2, font);
      item->setData(2, Qt::UserRole, QVariant::fromValue(line));

      QString content = line.content == "\n" ? "" : line.content;

      switch(line.type)
      {
      case GitDiffLine::diffType::ADD:
        item->setBackground(2, QColor::fromRgb(86, 244, 66));
        item->setForeground(2, Qt::black);
        item->setText(2, "+ " + content);
        firstInterestingItem = firstInterestingItem ? firstInterestingItem : item;
        ++_impl->nonTrivialLines;
        break;
      case GitDiffLine::diffType::REMOVE:
        item->setBackground(2, QColor::fromRgb(244, 66, 66));
        item->setForeground(2, Qt::black);
        item->setText(2, "- " + content);
        firstInterestingItem = firstInterestingItem ? firstInterestingItem : item;
        ++_impl->nonTrivialLines;
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

      col0Width = std::max(col0Width, metrics.size(Qt::TextSingleLine, QString::number(line.oldLine)).width());
      col1Width = std::max(col1Width, metrics.size(Qt::TextSingleLine, QString::number(line.newLine)).width());

      ui->treeWidget->addTopLevelItem(item);
    }

    ui->treeWidget->setColumnWidth(0, col0Width + 10);
    ui->treeWidget->setColumnWidth(1, col1Width + 10);

    ui->treeWidget->scrollToItem(firstInterestingItem);
  });
}
