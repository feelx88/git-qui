#ifndef TOOLBAREDITOR_HPP
#define TOOLBAREDITOR_HPP

#include <QSharedPointer>
#include <QWidget>

namespace Ui {
class ToolBarEditor;
}

class QToolBar;
struct ToolBarEditorPrivate;

class ToolBarEditor : public QWidget {
  Q_OBJECT
  friend struct ToolBarEditorPrivate;

public:
  explicit ToolBarEditor(QToolBar *toolbar);
  ~ToolBarEditor();

private:
  Ui::ToolBarEditor *ui;
  QSharedPointer<ToolBarEditorPrivate> _impl;
};

#endif // TOOLBAREDITOR_HPP
