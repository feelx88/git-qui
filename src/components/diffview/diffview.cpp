#include "diffview.h"
#include "ui_diffview.h"

#include <QMainWindow>
#include <QFontDatabase>

#include "gitinterface.hpp"
#include "gitdiffline.h"

struct DiffViewPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  void connectSignals(DiffView *_this)
  {
    _this->connect(gitInterface.get(), &GitInterface::fileDiffed, [=](const QString &path, QList<GitDiffLine> lines){
      _this->setWindowTitle(path);
      _this->ui->listWidget->clear();
      for (auto line : lines)
      {
        QListWidgetItem *item = new QListWidgetItem(_this->ui->listWidget);
        item->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

        switch(line.type)
        {
        case GitDiffLine::diffType::ADD:
          item->setForeground(Qt::green);
          item->setText("+ " + line.content);
          break;
        case GitDiffLine::diffType::REMOVE:
          item->setForeground(Qt::red);
          item->setText("- " + line.content);
          break;
        case GitDiffLine::diffType::CONTEXT:
          item->setForeground(Qt::gray);
          item->setText("  " + line.content);
          [[fallthrough]];
        case GitDiffLine::diffType::FILE_FOOTER:
        case GitDiffLine::diffType::FILE_HEADER:
        case GitDiffLine::diffType::HEADER:
        default:
          item->setText(line.content);
          break;
        }

        _this->ui->listWidget->addItem(item);
      }
    });
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
}

DiffView::~DiffView()
{
  delete ui;
}
