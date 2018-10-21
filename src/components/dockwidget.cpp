#include "dockwidget.hpp"
#include <QThread>

QMap<QString, DockWidget::RegistryEntry*> DockWidget::_registry;

DockWidget::DockWidget(QWidget *parent, const QString &id) :
QDockWidget(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName(id);
}

DockWidget::~DockWidget()
{
}

QList<DockWidget::RegistryEntry *> DockWidget::registeredDockWidgets()
{
  return _registry.values();
}

void DockWidget::create(QString className, QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString& id, const QVariant &configuration)
{
  RegistryEntry *entry = _registry.value(className, nullptr);

  if (entry)
  {
    entry->restorer(mainWindow, gitInterface, id, configuration);
  }
}

QVariant DockWidget::configuration()
{
  return QVariant();
}

bool DockWidget::doRegister(DockWidget::RegistryEntry *entry)
{
  _registry.insert(entry->id, entry);
  return true;
}
