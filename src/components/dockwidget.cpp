#include "dockwidget.hpp"
#include <QThread>
#include <QMainWindow>

QSharedPointer<QMap<QString, DockWidget::RegistryEntry*>> DockWidget::_registry;

DockWidget::DockWidget(QWidget *parent) :
QDockWidget(parent)
{
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

void DockWidget::create(QString className, QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString& id, const QVariant &configuration)
{
  RegistryEntry *entry = registry()->value(className, nullptr);

  if (entry)
  {
    DockWidget *widget = entry->factory(mainWindow, gitInterface);
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, widget);
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
  setTitleBarWidget(enabled ? nullptr : new QWidget(this));
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
