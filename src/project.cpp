#include "project.hpp"

#include <QDirIterator>
#include <QFileDialog>
#include <QRegularExpression>
#include <QSettings>
#include <algorithm>

#include "gitinterface.hpp"

struct ConfigurationKeys {
  static constexpr const char *NAME = "name";
  static constexpr const char *REPOSITORY_LIST = "repositoryList";
  static constexpr const char *CURRENT_REPOSITORY = "currentRepository";
  static constexpr const char *REPOSITORY_LIST_NAME = "name";
  static constexpr const char *REPOSITORY_LIST_PATH = "path";
  static constexpr const char *DOCK_WIDGET_CONFIGURATION =
      "dockWidgetConfiguration";
  static constexpr const char *AUTOFETCH_ENABLED = "autofetchEnabled";
};

struct ProjectPrivate {
  Project *_this = nullptr;
  QSettings *settings = nullptr;
  QString name = "";
  QList<QSharedPointer<GitInterface>> repositories;
  int currentRepository = 0;
  bool autoFetchEnabled = true;
  QList<QRegularExpression> ignoredSubdirectories = {
      QRegularExpression("/.*vendor.*/"),
      QRegularExpression("/.*node_modules.*/")};
  QSharedPointer<QObject> currentRepositoryContext = nullptr;

  ProjectPrivate(Project *project)
      : _this{project}, currentRepositoryContext{new QObject(_this)} {}

  void loadSettings() {
    name = settings->value(ConfigurationKeys::NAME).toString();
    QList<QVariantMap> list = qvariant_cast<QList<QVariantMap>>(
        settings->value(ConfigurationKeys::REPOSITORY_LIST));
    currentRepository = std::max(
        std::min(
            settings->value(ConfigurationKeys::CURRENT_REPOSITORY, 0).toInt(),
            list.size() - 1),
        0);
    autoFetchEnabled =
        settings->value(ConfigurationKeys::AUTOFETCH_ENABLED, true).toBool();

    for (const auto &entry : list) {
      repositories.append(QSharedPointer<GitInterface>(new GitInterface(
          entry.value(ConfigurationKeys::REPOSITORY_LIST_NAME).toString(),
          entry.value(ConfigurationKeys::REPOSITORY_LIST_PATH).toString())));
    }
  }

  void writeSettings() {
    if (settings) {
      settings->setValue(ConfigurationKeys::NAME, name);

      currentRepository =
          currentRepository > repositories.size() ? 0 : currentRepository;

      settings->setValue(ConfigurationKeys::CURRENT_REPOSITORY,
                         currentRepository);
      settings->setValue(ConfigurationKeys::AUTOFETCH_ENABLED,
                         autoFetchEnabled);

      QList<QVariantMap> list;

      for (const auto &repository : qAsConst(repositories)) {
        list << QVariantMap{
            {ConfigurationKeys::REPOSITORY_LIST_NAME, repository->name()},
            {ConfigurationKeys::REPOSITORY_LIST_PATH, repository->path()}};
      }

      settings->setValue(ConfigurationKeys::REPOSITORY_LIST,
                         QVariant::fromValue(list));
      settings->sync();
    }
  }

  void changeRepository() {
    currentRepositoryContext.reset(new QObject(_this));
    auto repo = repositories.at(currentRepository);
    emit _this->repositorySwitched(repo, currentRepositoryContext);
    repo->reload();
  }
};

Project::Project(const QString &fileName, QObject *parent) : Project(parent) {
  setFileName(fileName);
}

Project::Project(QObject *parent)
    : QObject(parent), _impl(new ProjectPrivate(this)) {}

Project::~Project() {}

QString Project::fileName() const {
  return _impl->settings ? _impl->settings->fileName() : "";
}

QString Project::name() const { return _impl->name; }

QList<QSharedPointer<GitInterface>> Project::repositoryList() const {
  return _impl->repositories;
}

QSharedPointer<GitInterface> Project::activeRepository() const {
  return _impl->repositories.at(std::max(
      std::min(_impl->repositories.size() - 1, _impl->currentRepository), 0));
}

QSharedPointer<QObject> Project::activeRepositoryContext() const {
  return _impl->currentRepositoryContext;
}

QSharedPointer<GitInterface>
Project::repositoryByName(const QString &name) const {
  auto it = std::find_if(_impl->repositories.begin(), _impl->repositories.end(),
                         [=](QSharedPointer<GitInterface> repository) {
                           return repository->name() == name;
                         });

  if (it != _impl->repositories.end()) {
    return *it;
  }
  return nullptr;
}

QVariantMap Project::dockWidgetConfiguration() const {
  return _impl->settings
      ->value(ConfigurationKeys::DOCK_WIDGET_CONFIGURATION, QVariantMap())
      .toMap();
}

void Project::setDockWidgetConfigurationEntry(const QString &key,
                                              QVariant value) {
  QVariantMap config = dockWidgetConfiguration();
  config.insert(key, value);
  _impl->settings->setValue(ConfigurationKeys::DOCK_WIDGET_CONFIGURATION,
                            config);
}

void Project::addRepository() {
  QString path = QFileDialog::getExistingDirectory(
      nullptr, QObject::tr("Select repository path"), QDir::current().path());

  if (!path.isNull()) {
    QDirIterator iterator(path, {".git"}, QDir::Dirs | QDir::Hidden,
                          QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
      QDir currentDir = QDir(iterator.next());
      currentDir.cdUp();

      bool directoryValid = true;

      for (const auto &regex : qAsConst(_impl->ignoredSubdirectories)) {
        if (regex.match(currentDir.path()).hasMatch()) {
          directoryValid = false;
          break;
        }
      }

      if (directoryValid) {
        auto repository = QSharedPointer<GitInterface>(new GitInterface(
            currentDir.dirName(), currentDir.absolutePath(), this));
        _impl->repositories.append(repository);
        emit repositoryAdded(repository);
      }
    }

    _impl->writeSettings();
  }
}

void Project::removeRepository(const int &index) {
  emit repositoryRemoved(_impl->repositories.at(index));
  _impl->repositories.removeAt(index);
  _impl->writeSettings();
}

void Project::setCurrentRepository(const int &index) {
  _impl->currentRepository = index;
  _impl->changeRepository();
}

void Project::setCurrentRepository(const QString &name) {
  auto found =
      std::find_if(_impl->repositories.begin(), _impl->repositories.end(),
                   [name](QSharedPointer<GitInterface> repository) {
                     return repository->name() == name;
                   });

  if (found == _impl->repositories.end()) {
    return;
  }

  _impl->currentRepository = _impl->repositories.indexOf(*found);
  _impl->changeRepository();
}

void Project::setName(const QString &name) {
  _impl->name = name;
  _impl->writeSettings();
}

void Project::setFileName(const QString &fileName) {
  if (!_impl->settings) {
    _impl->settings = new QSettings(fileName, QSettings::IniFormat, this);
    _impl->loadSettings();
    _impl->writeSettings();
  }
}

void Project::save() { _impl->writeSettings(); }

void Project::reloadAllRepositories() {
  for (auto &repository : repositoryList()) {
    repository->reload();
  }
}

bool Project::autoFetchEnabled() const { return _impl->autoFetchEnabled; }

void Project::setAutoFetchEnabled(bool enabled) {
  _impl->autoFetchEnabled = enabled;
  emit autoFetchChanged(enabled);
}
