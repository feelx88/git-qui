#include "dockwidget.hpp"
#include <QThread>

DockWidget *DockWidget::_instance = nullptr;

DockWidget::~DockWidget()
{
}

QList<DockWidget::RegistryEntry *> DockWidget::registeredDockWidgets()
{
  return instance()->_registry;
}

DockWidget *DockWidget::instance()
{
  if (nullptr == _instance)
  {
    _instance = new DockWidget;
  }
  return _instance;
}

bool DockWidget::doRegister(DockWidget::RegistryEntry *entry)
{
  instance()->_registry.append(entry);
  return true;
}
