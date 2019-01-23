#include "diffview.hpp"
#include "ui_diffview.h"

#include <QFontDatabase>
#include <QAction>

#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "gitdiffline.hpp"

struct DiffViewPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  bool unstaged = false;
  QString currentPath;
  QAction *fullFileDiffAction, *stageOrUnstageSelected, *resetSelected;
  int nonTrivialLines;

  void connectSignals(DiffView *_this)
  {
    auto clear = [=](){
      _this->setWindowTitle(_this->tr("Diff view"));
      _this->ui->treeWidget->clear();
      stageOrUnstageSelected->setVisible(false);
      resetSelected->setVisible(false);
    };

    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositorySwitched, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      _this->ui->treeWidget->clear();
      _this->disconnect(gitInterface.get(), &GitInterface::fileDiffed, _this, nullptr);

      gitInterface = newGitInterface;

      _this->connect(gitInterface.get(), &GitInterface::fileDiffed, _this, [=](const QString &path, QList<GitDiffLine> lines, bool unstaged){
        stageOrUnstageSelected->setVisible(true);
        resetSelected->setVisible(true);
        resetSelected->setEnabled(unstaged);

        currentPath = path;
        this->unstaged = unstaged;
        _this->setWindowTitle(path);
        stageOrUnstageSelected->setText(unstaged ?
          _this->tr("Stage selected lines") : _this->tr("Unstage selected lines"));
        _this->ui->treeWidget->clear();

        QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        QFontMetrics metrics(font);
        int col0Width = 0, col1Width = 0;
        QTreeWidgetItem *firstInterestingItem = nullptr;
        nonTrivialLines = 0;

        for (auto line : lines)
        {
          if (fullFileDiffAction->isChecked() && line.type == GitDiffLine::diffType::HEADER)
          {
            continue;
          }

          QTreeWidgetItem *item = new QTreeWidgetItem(_this->ui->treeWidget);
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
            ++nonTrivialLines;
            break;
          case GitDiffLine::diffType::REMOVE:
            item->setBackground(2, QColor::fromRgb(244, 66, 66));
            item->setForeground(2, Qt::black);
            item->setText(2, "- " + content);
            firstInterestingItem = firstInterestingItem ? firstInterestingItem : item;
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

          col0Width = std::max(col0Width, metrics.size(Qt::TextSingleLine, QString::number(line.oldLine)).width());
          col1Width = std::max(col1Width, metrics.size(Qt::TextSingleLine, QString::number(line.newLine)).width());

          _this->ui->treeWidget->addTopLevelItem(item);
        }

        _this->ui->treeWidget->setColumnWidth(0, col0Width + 10);
        _this->ui->treeWidget->setColumnWidth(1, col1Width + 10);

        _this->ui->treeWidget->scrollToItem(firstInterestingItem);
      });
    });

    _this->connect(_this->ui->treeWidget, &QTreeWidget::itemDoubleClicked, _this, [=]{
      stageOrUnstage(_this);
    });
  }

  void addActions(DiffView *_this)
  {
    fullFileDiffAction = new QAction(_this->tr("Full file diff"), _this);
    fullFileDiffAction->setCheckable(true);
    _this->addAction(fullFileDiffAction);
    _this->connect(fullFileDiffAction, &QAction::toggled, _this, [=](bool checked){
      gitInterface->setFullFileDiff(checked);
      gitInterface->diffFile(unstaged, currentPath);
    });
  }

  void addContextMenuActions(DiffView *_this)
  {
    stageOrUnstageSelected = new QAction(_this);
    _this->connect(stageOrUnstageSelected, &QAction::triggered, _this, [=]{
      stageOrUnstage(_this);
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

  void stageOrUnstage(DiffView *_this)
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

DiffView::DiffView(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
  DockWidget(parent),
  ui(new Ui::DiffView),
  _impl(new DiffViewPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;

  _impl->connectSignals(this);
  _impl->addActions(this);
  _impl->addContextMenuActions(this);
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
