#ifndef DOCKWIDGET_HPP
#define DOCKWIDGET_HPP

#include "gitinterface.hpp"
#include <QFuture>
#include <QList>
#include <QMainWindow>
#include <QUuid>

#include <DockManager.h>
#include <DockWidget.h>

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

struct DockWidgetPrivate;

class DockWidget : public ads::CDockWidget {
  Q_OBJECT
public:
  friend struct DockWidgetPrivate;

  struct RegistryEntry {
    QString id;
    QString name;
    std::function<DockWidget *(MainWindow *)> factory;
  };

  inline static const char *CHILD_WIDGET_AUTO_DISABLE_PROPERTY_NAME =
      "disableDuringRepositoryAction";
  inline static int CHILD_WIDGET_AUTO_DISABLE_DEBOUNCE_TIME = 500;
  inline static QList<GitInterface::ActionTag> NON_LOCKING_ACTIONS = {
      GitInterface::ActionTag::RELOAD, GitInterface::ActionTag::GIT_STATUS,
      GitInterface::ActionTag::GIT_LOG, GitInterface::ActionTag::GIT_FETCH,
      GitInterface::ActionTag::GIT_DIFF};

  virtual ~DockWidget() override;
  static QList<RegistryEntry *> registeredDockWidgets();
  static DockWidget *create(QString className, MainWindow *mainWindow,
                            ads::CDockManager *container,
                            const QString &id = QUuid::createUuid().toString(),
                            const QVariant &configuration = QVariant(),
                            ads::DockWidgetArea area = ads::TopDockWidgetArea);

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
  virtual void onRepositoryAdded(QSharedPointer<GitInterface> gitInterface);
  virtual void
  onRepositorySwitched(QSharedPointer<GitInterface> newGitInterface,
                       QSharedPointer<QObject> activeRepositoryContext);
  virtual void onRepositoryRemoved(QSharedPointer<GitInterface> gitInterface);
  virtual void onError(const QString &, GitInterface::ActionTag,
                       GitInterface::ErrorType, bool consoleOutput = false,
                       QVariantMap context = {});

private:
  static QSharedPointer<QMap<QString, RegistryEntry *>> registry();
  static QSharedPointer<QMap<QString, RegistryEntry *>> _registry;

  MainWindow *_mainWindow;
  QScopedPointer<DockWidgetPrivate> _impl;
};

#endif // DOCKWIDGET_HPP
