#include "dockwidget.hpp"
#include <QThread>

QSharedPointer<QMap<QString, DockWidget::RegistryEntry*>> DockWidget::_registry;

DockWidget::DockWidget(QWidget *parent, const QString &id) :
QDockWidget(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName(id);
  setFeatures(DockWidgetClosable | DockWidgetMovable);
  setContextMenuPolicy(Qt::CustomContextMenu);
}

DockWidget::~DockWidget()
{
}

QList<DockWidget::RegistryEntry *> DockWidget::registeredDockWidgets()
{
  return registry()->values();
}

void DockWidget::create(QString className, QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString& id, const QVariant &configuration)
{
  RegistryEntry *entry = registry()->value(className, nullptr);

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
  registry()->insert(entry->id, entry);
  return true;
}

QSharedPointer<QMap<QString, DockWidget::RegistryEntry *> > DockWidget::registry()
{
  if (_registry.isNull())
  {
    _registry.reset(new QMap<QString, RegistryEntry*>);
  }
  return _registry;
}
