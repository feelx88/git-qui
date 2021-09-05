#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

#include "gitinterface.hpp"
#include <QDockWidget>
#include <QFuture>
#include <QList>
#include <QMainWindow>
#include <QUuid>

#define DOCK_WIDGET                                                            \
protected:                                                                     \
  virtual QString widgetName() const override;                                 \
                                                                               \
private:                                                                       \
  static bool __registryHelper;                                                \
  static DockWidget::RegistryEntry *registryEntry();

#define DOCK_WIDGET_IMPL(name, displayName)                                    \
  bool name::__registryHelper = DockWidget::doRegister(name::registryEntry()); \
  QString name::widgetName() const {                                           \
    return name::staticMetaObject.className();                                 \
  }                                                                            \
  DockWidget::RegistryEntry *name::registryEntry() {                           \
    return new DockWidget::RegistryEntry{                                      \
        name::staticMetaObject.className(), displayName,                       \
        [](MainWindow *mainWindow) { return new name(mainWindow); }};          \
  }

class MainWindow;
class Core;
class Project;

class DockWidget : public QDockWidget {
public:
  struct RegistryEntry {
    QString id;
    QString name;
    std::function<DockWidget *(MainWindow *)> factory;
  };

  inline static const char *CHILD_WIDGET_AUTO_DISABLE_PROPERTY_NAME =
      "disableDuringRepositoryAction";

  virtual ~DockWidget() override;
  static QList<RegistryEntry *> registeredDockWidgets();
  static DockWidget *create(QString className, MainWindow *mainWindow,
                            QMainWindow *container,
                            const QString &id = QUuid::createUuid().toString(),
                            const QVariant &configuration = QVariant());

  virtual QVariant configuration();
  virtual void configure(const QVariant &configuration);

  void setEditModeEnabled(bool enabled);

  MainWindow *mainWindow();
  Core *core();
  Project *project();

  template <class T, class S>
  QMetaObject::Connection connectMainWindowSignal(T signal, S slot);
  template <class T, class S>
  QMetaObject::Connection connectCoreSignal(T signal, S slot);
  template <class T, class S>
  QMetaObject::Connection connectProjectSignal(T signal, S slot);

protected:
  DockWidget(MainWindow *mainWindow);
  void init();
  static bool doRegister(RegistryEntry *entry);

  virtual QString widgetName() const = 0;

  virtual QVariantMap getProjectSpecificConfiguration();

  virtual void onProjectSwitched(Project *newProject);
  virtual void
  onProjectSpecificConfigurationLoaded(const QVariantMap &configuration);
  virtual void onRepositoryAdded(GitInterface *gitInterface);
  virtual void onRepositorySwitched(GitInterface *newGitInterface,
                                    QObject *activeRepositoryContext);
  virtual void onRepositoryRemoved(GitInterface *gitInterface);
  virtual void onError(const QString &, GitInterface::ActionTag,
                       GitInterface::ErrorType);

private:
  static QSharedPointer<QMap<QString, RegistryEntry *>> registry();
  static QSharedPointer<QMap<QString, RegistryEntry *>> _registry;

  MainWindow *_mainWindow;

  void setChildWidgetsDisabledState(bool disabled);
};

#endif // DOCKWIDGET_HPP
