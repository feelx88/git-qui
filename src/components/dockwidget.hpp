#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

#include <QFuture>

class QMainWindow;
class GitInterface;

#define DOCK_WIDGET \
  private: \
    static bool __registryHelper; \
    static DockWidget::RegistryEntry* registryEntry();

#define DOCK_WIDGET_IMPL(name, displayName, initializer, restorer) \
  bool name::__registryHelper = DockWidget::doRegister(name::registryEntry()); \
  DockWidget::RegistryEntry *name::registryEntry() \
  { \
  return new DockWidget::RegistryEntry { \
    name::staticMetaObject.className(), \
    displayName, \
    initializer, \
    restorer, \
  }; \
}

#include <QDockWidget>
#include <QList>

class DockWidget : public QDockWidget
{
public:
  struct RegistryEntry
  {
    QString id;
    QString name;
    std::function<void(QMainWindow*, const QSharedPointer<GitInterface>&)> initializer;
    std::function<void(QMainWindow*, const QSharedPointer<GitInterface>&, const QString &, const QVariant&)> restorer;
  };

  DockWidget(QWidget *parent = nullptr, const QString &id = QVariant(qrand()).toString());
  virtual ~DockWidget() override;
  static QList<RegistryEntry*> registeredDockWidgets();
  static void create(QString className, QMainWindow* mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &configuration);

  virtual QVariant configuration();

protected:
  static bool doRegister(RegistryEntry *entry);

private:
  static QMap<QString, RegistryEntry*> _registry;
};

#endif // DOCKWIDGET_HPP
