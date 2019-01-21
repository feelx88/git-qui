#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

#include <QFuture>
#include <QMainWindow>
#include <gitinterface.hpp>

#define DOCK_WIDGET \
  private: \
    static bool __registryHelper; \
    static DockWidget::RegistryEntry* registryEntry();

#define DOCK_WIDGET_IMPL(name, displayName) \
  bool name::__registryHelper = DockWidget::doRegister(name::registryEntry()); \
  DockWidget::RegistryEntry *name::registryEntry() \
  { \
  return new DockWidget::RegistryEntry { \
    name::staticMetaObject.className(), \
    displayName, \
    [](QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface){return new name(mainWindow, gitInterface);} \
  }; \
}

#include <QDockWidget>
#include <QList>
#include <QUuid>

class DockWidget : public QDockWidget
{
public:
  struct RegistryEntry
  {
    QString id;
    QString name;
    std::function<DockWidget*(QMainWindow*, const QSharedPointer<GitInterface>&)> factory;
  };

  virtual ~DockWidget() override;
  static QList<RegistryEntry*> registeredDockWidgets();
  static void create(
    QString className,
    QMainWindow* mainWindow,
    const QSharedPointer<GitInterface> &gitInterface,
    const QString &id = QUuid::createUuid().toString(),
    const QVariant &configuration = QVariant()
  );

  virtual QVariant configuration();
  virtual void configure(const QVariant &configuration);

  void setEditModeEnabled(bool enabled);

protected:
  DockWidget(QWidget *parent = nullptr);
  static bool doRegister(RegistryEntry *entry);

private:
  static QSharedPointer<QMap<QString, RegistryEntry*>> registry();
  static QSharedPointer<QMap<QString, RegistryEntry*>> _registry;
};

#endif // DOCKWIDGET_HPP
