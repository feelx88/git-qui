#include "dockwidget.hpp"

#include "core.hpp"
#include "gitinterface.hpp"
#include "mainwindow.hpp"
#include "project.hpp"

#include <QApplication>

QSharedPointer<QMap<QString, DockWidget::RegistryEntry *>>
    DockWidget::_registry;

DockWidget::DockWidget(MainWindow *mainWindow) : QDockWidget(mainWindow) {
  _mainWindow = mainWindow;
  setAttribute(Qt::WA_DeleteOnClose);
  setFeatures(DockWidgetClosable | DockWidgetMovable);
  setContextMenuPolicy(Qt::CustomContextMenu);
}

void DockWidget::init() {
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

  for (auto repository : project()->repositoryList()) {
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
                               QMainWindow *container, const QString &id,
                               const QVariant &configuration) {
  RegistryEntry *entry = registry()->value(className, nullptr);

  if (entry) {
    DockWidget *widget = entry->factory(mainWindow);
    container->addDockWidget(Qt::TopDockWidgetArea, widget);
    widget->setObjectName(id);
    widget->configure(configuration);
    widget->init();

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

void DockWidget::onRepositoryAdded(GitInterface *gitInterface) {
  connect(gitInterface, &GitInterface::error, this, &DockWidget::onError);
}

void DockWidget::onRepositorySwitched(GitInterface *newGitInterface,
                                      QObject *activeRepositoryContext) {
  connect(newGitInterface, &GitInterface::actionStarted,
          activeRepositoryContext,
          std::bind(std::mem_fn(&DockWidget::setChildWidgetsDisabledState),
                    this, true));

  connect(newGitInterface, &GitInterface::actionFinished,
          activeRepositoryContext,
          std::bind(std::mem_fn(&DockWidget::setChildWidgetsDisabledState),
                    this, false));
}

void DockWidget::onRepositoryRemoved(GitInterface *gitInterface) {
  disconnect(gitInterface, &GitInterface::error, this, &DockWidget::onError);
}

void DockWidget::onError(const QString &, ActionTag, ErrorType) {}

QSharedPointer<QMap<QString, DockWidget::RegistryEntry *>>
DockWidget::registry() {
  if (_registry.isNull()) {
    _registry.reset(new QMap<QString, RegistryEntry *>);
  }
  return _registry;
}

void DockWidget::setChildWidgetsDisabledState(bool disabled) {
  for (const auto &child : findChildren<QWidget *>()) {
    if (child->property(CHILD_WIDGET_AUTO_DISABLE_PROPERTY_NAME).toBool()) {
      child->setDisabled(disabled);
    }
  }
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
