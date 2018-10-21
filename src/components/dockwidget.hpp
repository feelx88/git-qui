#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

class QDockWidget;
class GitInterface;

#define DOCK_WIDGET \
  private: \
    static bool __registryHelper; \
    static DockWidget::RegistryEntry* registryEntry();

#define DOCK_WIDGET_IMPL(name) \
  bool name::__registryHelper = DockWidget::doRegister(name::registryEntry()); \
  DockWidget::RegistryEntry *name::registryEntry()

#include <QList>

class DockWidget
{
public:
  struct RegistryEntry
  {
    QString name;
    std::function<QDockWidget*(QWidget*, QSharedPointer<GitInterface>)> initializer;
  };

  virtual ~DockWidget();
  static QList<RegistryEntry*> registeredDockWidgets();

protected:
  static DockWidget *instance();
  static bool doRegister(RegistryEntry *entry);

private:
  QList<RegistryEntry*> _registry;
  static DockWidget *_instance;
};

#endif // DOCKWIDGET_HPP
