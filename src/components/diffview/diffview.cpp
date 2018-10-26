#include "diffview.h"
#include "ui_diffview.h"

#include <QMainWindow>
#include <QFontDatabase>
#include <QAction>

#include "gitinterface.hpp"
#include "gitdiffline.h"

struct DiffViewPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  bool unstaged = false;

  void connectSignals(DiffView *_this)
  {
    auto clear = [=](){
      _this->setWindowTitle(_this->tr("Diff editor"));
      _this->ui->treeWidget->clear();
    };

    _this->connect(gitInterface.get(), &GitInterface::stagingAreaChanged, _this, clear);
    _this->connect(gitInterface.get(), &GitInterface::nonStagingAreaChanged, _this, clear);

    _this->connect(gitInterface.get(), &GitInterface::fileDiffed, _this, [=](const QString &path, QList<GitDiffLine> lines, bool unstaged){
      this->unstaged = unstaged;
      _this->setWindowTitle(path);
      _this->ui->treeWidget->clear();

      QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
      QFontMetrics metrics(font);
      int col0Width = 0, col1Width = 0;

      for (auto line : lines)
      {
        QTreeWidgetItem *item = new QTreeWidgetItem(_this->ui->treeWidget);
        item->setFont(2, font);
        item->setData(2, Qt::UserRole, QVariant::fromValue(line));

        QString content = line.content == "\n" ? "" : line.content;

        switch(line.type)
        {
        case GitDiffLine::diffType::ADD:
          item->setBackground(2, Qt::green);
          item->setForeground(2, Qt::black);
          item->setText(2, "+ " + content);
          break;
        case GitDiffLine::diffType::REMOVE:
          item->setBackground(2, Qt::red);
          item->setForeground(2, Qt::black);
          item->setText(2, "- " + content);
          break;
        case GitDiffLine::diffType::CONTEXT:
          item->setForeground(2, Qt::gray);
          item->setText(2, "  " + content);
          break;
        case GitDiffLine::diffType::FILE_FOOTER:
        case GitDiffLine::diffType::FILE_HEADER:
        case GitDiffLine::diffType::HEADER:
        default:
          item->setBackground(2, Qt::gray);
          item->setText(2, content);
          break;
        }

        item->setText(0, line.oldLine > 0 ? QString::number(line.oldLine) : "");
        item->setText(1, line.newLine > 0 ? QString::number(line.newLine) : "");
        item->setBackground(0, Qt::gray);
        item->setBackground(1, Qt::gray);

        col0Width = std::max(col0Width, metrics.width(QString::number(line.oldLine)));
        col1Width = std::max(col1Width, metrics.width(QString::number(line.newLine)));

        _this->ui->treeWidget->addTopLevelItem(item);

      }

      _this->ui->treeWidget->setColumnWidth(0, col0Width + 10);
      _this->ui->treeWidget->setColumnWidth(1, col1Width + 10);
    });
  }

  void addContextMenuActions(DiffView *_this)
  {
    QAction *stageOrUnstageSelected = new QAction(_this->tr("[Un]Stage selected lines"));
    _this->connect(stageOrUnstageSelected, &QAction::triggered, _this, [=]{
      QList<GitDiffLine> lines;
      for (auto item : _this->ui->treeWidget->selectedItems())
      {
        lines.append(item->data(2, Qt::UserRole).value<GitDiffLine>());
      }
      gitInterface->addLines(lines, unstaged);
    });

    _this->ui->treeWidget->addActions(QList<QAction*>() << stageOrUnstageSelected);
  }

  static void initialize(QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new DiffView(mainWindow, gitInterface));
  }

  static void restore(QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new DiffView(mainWindow, gitInterface));
    mainWindow->setObjectName(id);
  }
};

DOCK_WIDGET_IMPL(
    DiffView,
    tr("Diff View"),
    &DiffViewPrivate::initialize,
    &DiffViewPrivate::restore
)

DiffView::DiffView(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
  DockWidget(parent),
  ui(new Ui::DiffView),
  _impl(new DiffViewPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface = gitInterface;

  _impl->connectSignals(this);
  _impl->addContextMenuActions(this);
}

DiffView::~DiffView()
{
  delete ui;
}
