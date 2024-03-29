#include "toolbareditor.hpp"
#include "ui_toolbareditor.h"

#include <QDebug>
#include <QToolBar>
#include <QTreeWidget>

#include "toolbaractions.hpp"
#include "treewidgetitem.hpp"

struct ToolBarEditorPrivate {
  void createItem(QTreeWidget *widget, QAction *action) {
    if (action->data().isNull()) {
      auto item = new TreeWidgetItem(widget, {ToolBarEditor::tr("Separator")});
      widget->addTopLevelItem(item);
    } else {
      auto item = new TreeWidgetItem(widget, {action->text()});
      item->setData(0, Qt::UserRole, action->data().toString());
      item->setIcon(0, action->icon());
      widget->addTopLevelItem(item);
    }
  }
};

ToolBarEditor::ToolBarEditor(QToolBar *toolbar)
    : QWidget(nullptr), ui(new Ui::ToolBarEditor),
      _impl(new ToolBarEditorPrivate) {
  ui->setupUi(this);

  connect(ui->treeWidget_2, &QTreeWidget::itemSelectionChanged, this,
          [=, this] {
            auto items = ui->treeWidget_2->selectedItems();

            ui->toolButtonLeft->setEnabled(!items.isEmpty());
            ui->toolButtonUp->setEnabled(
                !items.isEmpty() &&
                ui->treeWidget_2->indexOfTopLevelItem(items.first()) > 0);
            ui->toolButtonDown->setEnabled(
                !items.isEmpty() &&
                ui->treeWidget_2->indexOfTopLevelItem(items.first()) <
                    ui->treeWidget_2->topLevelItemCount() - 1);
          });

  connect(ui->treeWidget, &QTreeWidget::itemSelectionChanged, this, [=, this] {
    ui->toolButtonRight->setEnabled(!ui->treeWidget->selectedItems().empty());
  });

  auto addSlot = [=, this] {
    QTreeWidgetItem *item = ui->treeWidget->selectedItems().first();
    QAction *action =
        ToolBarActions::byId(item->data(0, Qt::UserRole).toString());
    _impl->createItem(ui->treeWidget_2, action);
    toolbar->addAction(action);
    delete item;
  };
  connect(ui->toolButtonRight, &QToolButton::clicked, this, addSlot);
  connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, addSlot);

  auto removeSlot = [=, this] {
    QTreeWidgetItem *item = ui->treeWidget_2->selectedItems().first();
    auto data = item->data(0, Qt::UserRole);

    if (!data.isNull()) {
      QAction *action = ToolBarActions::byId(data.toString());
      _impl->createItem(ui->treeWidget, action);
      toolbar->removeAction(action);
    } else {
      toolbar->removeAction(
          toolbar->actions().at(ui->treeWidget_2->currentIndex().row()));
    }
    delete item;
  };
  connect(ui->toolButtonLeft, &QToolButton::clicked, this, removeSlot);
  connect(ui->treeWidget_2, &QTreeWidget::itemDoubleClicked, this, removeSlot);

  connect(ui->toolButtonUp, &QToolButton::clicked, this, [=, this] {
    int index =
        ui->treeWidget_2->indexOfTopLevelItem(ui->treeWidget_2->currentItem());

    if (index == 0) {
      return;
    }

    auto item = ui->treeWidget_2->takeTopLevelItem(index);
    ui->treeWidget_2->insertTopLevelItem(index - 1, item);
    ui->treeWidget_2->setCurrentItem(item);

    auto action = toolbar->actions().at(index);
    toolbar->removeAction(action);
    toolbar->insertAction(toolbar->actions().at(index - 1), action);
  });

  connect(ui->toolButtonDown, &QToolButton::clicked, this, [=, this] {
    int index =
        ui->treeWidget_2->indexOfTopLevelItem(ui->treeWidget_2->currentItem());

    if (index >= ui->treeWidget_2->topLevelItemCount() - 1) {
      return;
    }

    auto item = ui->treeWidget_2->takeTopLevelItem(index);
    ui->treeWidget_2->insertTopLevelItem(index + 1, item);
    ui->treeWidget_2->setCurrentItem(item);

    auto action = toolbar->actions().at(index);
    toolbar->removeAction(action);
    toolbar->insertAction(index == ui->treeWidget_2->topLevelItemCount() - 2
                              ? nullptr
                              : toolbar->actions().at(index + 1),
                          action);
  });

  connect(ui->toolButtonSeparator, &QToolButton::clicked, this, [=, this] {
    _impl->createItem(ui->treeWidget_2, new QAction());
    toolbar->addSeparator();
  });

  auto actions = toolbar->actions();
  for (auto action : actions) {
    _impl->createItem(ui->treeWidget_2, action);
  }

  for (auto &[id, action] : ToolBarActions::all().toStdMap()) {
    if (ui->treeWidget_2->findItems(action->text(), Qt::MatchExactly, 0)
            .size() == 0) {
      _impl->createItem(ui->treeWidget, action);
    }
  }
}

ToolBarEditor::~ToolBarEditor() { delete ui; }
