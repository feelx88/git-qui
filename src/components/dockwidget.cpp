#include "dockwidget.hpp"

#include "mainwindow.hpp"

QSharedPointer<QMap<QString, DockWidget::RegistryEntry*>> DockWidget::_registry;

DockWidget::DockWidget(MainWindow *mainWindow) :
QDockWidget(mainWindow)
{
  _mainWindow = mainWindow;
  setAttribute(Qt::WA_DeleteOnClose);
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

void DockWidget::create(
  QString className,
  MainWindow *mainWindow,
  QMainWindow *container,
  GitInterface *gitInterface,
  const QString& id,
  const QVariant &configuration
)
{
  RegistryEntry *entry = registry()->value(className, nullptr);

  if (entry)
  {
    DockWidget *widget = entry->factory(mainWindow, gitInterface);
    container->addDockWidget(Qt::TopDockWidgetArea, widget);
    widget->setObjectName(id);
    widget->configure(configuration);
  }
}

QVariant DockWidget::configuration()
{
  return QVariant();
}

void DockWidget::configure(const QVariant &)
{
}

void DockWidget::setEditModeEnabled(bool enabled)
{
  setFeatures(enabled ?
    features() | DockWidgetClosable | DockWidgetMovable :
    features() & ~DockWidgetClosable & ~DockWidgetMovable);
}

MainWindow *DockWidget::mainWindow()
{
  return _mainWindow;
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
