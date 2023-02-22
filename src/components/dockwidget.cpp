#include "dockwidget.hpp"

#include "core.hpp"
#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "project.hpp"

#include <QApplication>
#include <QTimer>

struct DockWidgetPrivate {
  QTimer *uiLockTimer;
  DockWidget *_this;

  DockWidgetPrivate(DockWidget *_this) : _this(_this) {
    uiLockTimer = new QTimer(_this);
    uiLockTimer->setSingleShot(true);
    uiLockTimer->setInterval(
        DockWidget::CHILD_WIDGET_AUTO_DISABLE_DEBOUNCE_TIME);

    QObject::connect(uiLockTimer, &QTimer::timeout, _this,
                     [this] { setChildWidgetsDisabledState(true); });
  }

  void startUiLockTimer(const GitInterface::ActionTag &actionTag) {
    if (!DockWidget::NON_LOCKING_ACTIONS.contains(actionTag)) {
      uiLockTimer->start();
    }
  }

  void setChildWidgetsDisabledState(bool disabled) {
    return;
    if (!disabled && uiLockTimer) {
      uiLockTimer->stop();
    }

    auto children = _this->findChildren<QWidget *>();
    for (const auto &child : children) {
      if (child->property(DockWidget::CHILD_WIDGET_AUTO_DISABLE_PROPERTY_NAME)
              .toBool()) {
        child->setDisabled(disabled);
      }
    }
  }
};

QSharedPointer<QMap<QString, DockWidget::RegistryEntry *>>
    DockWidget::_registry;

DockWidget::DockWidget(MainWindow *mainWindow)
    : ads::CDockWidget("", mainWindow), _impl(new DockWidgetPrivate(this)) {
  _mainWindow = mainWindow;
  setFeature(ads::CDockWidget::DockWidgetAlwaysCloseAndDelete, true);
  setContextMenuPolicy(Qt::CustomContextMenu);
}

void DockWidget::init() {
  setWidget(findChild<QFrame *>());

  connectCoreSignal(&Core::beforeProjectChanged, [&](Project *oldProject) {
    oldProject->setDockWidgetConfigurationEntry(
        widgetName(), getProjectSpecificConfiguration());
  });

  connectCoreSignal(&Core::projectChanged, &DockWidget::onProjectSwitched);

  connect(qApp, &QApplication::aboutToQuit, this, [&] {
    project()->setDockWidgetConfigurationEntry(
        widgetName(), getProjectSpecificConfiguration());
  });

  onProjectSwitched(project());

  auto repositories = project()->repositoryList();
  for (const auto &repository : repositories) {
    onRepositoryAdded(repository);
  }

  onRepositorySwitched(project()->activeRepository(),
                       project()->activeRepositoryContext());
}

DockWidget::~DockWidget() {}

QList<DockWidget::RegistryEntry *> DockWidget::registeredDockWidgets() {
  return registry()->values();
}

DockWidget *DockWidget::create(QString className, MainWindow *mainWindow,
                               ads::CDockManager *container, const QString &id,
                               const QVariant &configuration) {
  RegistryEntry *entry = registry()->value(className, nullptr);

  if (entry) {
    DockWidget *widget = entry->factory(mainWindow);
    widget->setObjectName(id);
    widget->configure(configuration);
    widget->init();
    container->addDockWidget(ads::CenterDockWidgetArea, widget);

    return widget;
  }

  return nullptr;
}

QVariant DockWidget::configuration() { return QVariant(); }

void DockWidget::configure(const QVariant &) {}

void DockWidget::setEditModeEnabled(bool enabled) {
  setFeatures(enabled ? features() | DockWidgetClosable | DockWidgetMovable
                      : features() & ~DockWidgetClosable & ~DockWidgetMovable);
}

MainWindow *DockWidget::mainWindow() { return _mainWindow; }

Core *DockWidget::core() { return _mainWindow->core(); }

Project *DockWidget::project() { return core()->project(); }

bool DockWidget::doRegister(DockWidget::RegistryEntry *entry) {
  registry()->insert(entry->id, entry);
  return true;
}

QVariantMap DockWidget::getProjectSpecificConfiguration() {
  return QVariantMap();
}

void DockWidget::onProjectSwitched(Project *newProject) {
  connectProjectSignal(&Project::repositoryAdded,
                       &DockWidget::onRepositoryAdded);
  connectProjectSignal(&Project::repositorySwitched,
                       &DockWidget::onRepositorySwitched);
  connectProjectSignal(&Project::repositoryRemoved,
                       &DockWidget::onRepositoryRemoved);

  onRepositorySwitched(newProject->activeRepository(),
                       project()->activeRepositoryContext());

  onProjectSpecificConfigurationLoaded(
      newProject->dockWidgetConfiguration().value(widgetName()).toMap());
}

void DockWidget::onProjectSpecificConfigurationLoaded(const QVariantMap &) {}

void DockWidget::onRepositoryAdded(QSharedPointer<GitInterface> gitInterface) {
  connect(gitInterface.get(), &GitInterface::error, this, &DockWidget::onError);
}

void DockWidget::onRepositorySwitched(
    QSharedPointer<GitInterface> newGitInterface,
    QSharedPointer<QObject> activeRepositoryContext) {
  _impl->setChildWidgetsDisabledState(newGitInterface->actionRunning());

  connect(newGitInterface.get(), &GitInterface::actionStarted,
          activeRepositoryContext.get(),
          [this](const GitInterface::ActionTag &actionTag) {
            _impl->startUiLockTimer(actionTag);
          });

  connect(newGitInterface.get(), &GitInterface::actionFinished,
          activeRepositoryContext.get(),
          [this] { _impl->setChildWidgetsDisabledState(false); });
}

void DockWidget::onRepositoryRemoved(
    QSharedPointer<GitInterface> gitInterface) {
  disconnect(gitInterface.get(), &GitInterface::error, this,
             &DockWidget::onError);
}

void DockWidget::onError(const QString &, GitInterface::ActionTag,
                         GitInterface::ErrorType) {}

QSharedPointer<QMap<QString, DockWidget::RegistryEntry *>>
DockWidget::registry() {
  if (_registry.isNull()) {
    _registry.reset(new QMap<QString, RegistryEntry *>);
  }
  return _registry;
}

template <class T, class S>
QMetaObject::Connection DockWidget::connectMainWindowSignal(T signal, S slot) {
  return connect(mainWindow(), signal, this, slot);
}

template <class T, class S>
QMetaObject::Connection DockWidget::connectCoreSignal(T signal, S slot) {
  return connect(core(), signal, this, slot);
}

template <class T, class S>
QMetaObject::Connection DockWidget::connectProjectSignal(T signal, S slot) {
  return connect(project(), signal, this, slot);
}
