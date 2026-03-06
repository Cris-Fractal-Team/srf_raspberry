#include "app/LectorConfig.h"

#include <utility>
#include <vector>

LectorConfig& LectorConfig::getInstance()
{
    static LectorConfig singletonInstance;
    return singletonInstance;
}

void LectorConfig::initFromFile(const std::string& configurationFilePath, bool shouldConvertParameterNamesToLowercase)
{
    std::shared_ptr<GHashMap> loadedConfigurationMap =
        leeArchivoConfig(configurationFilePath, shouldConvertParameterNamesToLowercase);

    if (!loadedConfigurationMap) {
        loadedConfigurationMap = std::make_shared<GHashMap>();
    }

    std::lock_guard<std::mutex> mutexGuard(mtx_);
    params_ = std::move(loadedConfigurationMap);
    path_ = configurationFilePath;
    convierteNombresLowerCase_ = shouldConvertParameterNamesToLowercase;
    initialized_ = true;
}

void LectorConfig::reload()
{
    std::string configurationFilePath;
    bool shouldConvertParameterNamesToLowercase = false;

    {
        std::lock_guard<std::mutex> mutexGuard(mtx_);
        if (!initialized_ || path_.empty()) {
            return;
        }
        configurationFilePath = path_;
        shouldConvertParameterNamesToLowercase = convierteNombresLowerCase_;
    }

    std::shared_ptr<GHashMap> reloadedConfigurationMap =
        leeArchivoConfig(configurationFilePath, shouldConvertParameterNamesToLowercase);

    if (!reloadedConfigurationMap) {
        reloadedConfigurationMap = std::make_shared<GHashMap>();
    }

    {
        std::lock_guard<std::mutex> mutexGuard(mtx_);
        params_ = std::move(reloadedConfigurationMap);
    }
}

void LectorConfig::save(const std::string& outputFilePath)
{
    std::shared_ptr<GHashMap> snapshotConfigurationMap;
    std::string targetFilePath;

    {
        std::lock_guard<std::mutex> mutexGuard(mtx_);
        snapshotConfigurationMap = params_ ? params_ : std::make_shared<GHashMap>();
        targetFilePath = outputFilePath.empty() ? path_ : outputFilePath;
    }

    if (targetFilePath.empty()) {
        return;
    }

    guardaArchivoConfig(targetFilePath, snapshotConfigurationMap);
}

std::shared_ptr<GHashMap> LectorConfig::getParams() const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    return params_;
}

void LectorConfig::setParams(const std::shared_ptr<GHashMap>& newConfigurationMap)
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    params_ = newConfigurationMap ? newConfigurationMap : std::make_shared<GHashMap>();
    initialized_ = true;
}

void LectorConfig::mergeInto(std::shared_ptr<GHashMap>& targetConfigurationMap,
                            const std::shared_ptr<GHashMap>& sourceConfigurationMap)
{
    if (!targetConfigurationMap) {
        targetConfigurationMap = std::make_shared<GHashMap>();
    }
    if (!sourceConfigurationMap) {
        return;
    }

    const std::vector<std::string> sourceKeys = sourceConfigurationMap->getLstClaves();
    for (const std::string& parameterKey : sourceKeys) {
        targetConfigurationMap->put(parameterKey, sourceConfigurationMap->get(parameterKey));
    }
}

void LectorConfig::applyOverrides(const std::shared_ptr<GHashMap>& overrideConfigurationMap)
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    mergeInto(params_, overrideConfigurationMap);
    initialized_ = true;
}

bool LectorConfig::hasKey(const std::string& parameterKey) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    return params_ && params_->hasKey(parameterKey);
}

std::string LectorConfig::getString(const std::string& parameterKey, const std::string& defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_ || !params_->hasKey(parameterKey)) {
        return defaultValue;
    }

    const std::string storedValue = params_->getString(parameterKey);
    return storedValue.empty() ? defaultValue : storedValue;
}

int LectorConfig::getInt(const std::string& parameterKey, int defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getInt(parameterKey, defaultValue);
}

long LectorConfig::getLong(const std::string& parameterKey, long defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getLong(parameterKey, defaultValue);
}

long long LectorConfig::getLongLong(const std::string& parameterKey, long long defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getLongLong(parameterKey, defaultValue);
}

double LectorConfig::getDouble(const std::string& parameterKey, double defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getDouble(parameterKey, defaultValue);
}

long LectorConfig::getStringLong(const std::string& parameterKey, long defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getStringLong(parameterKey, defaultValue);
}

double LectorConfig::getStringDouble(const std::string& parameterKey, double defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getStringDouble(parameterKey, defaultValue);
}

bool LectorConfig::getStringBool(const std::string& parameterKey, bool defaultValue) const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    if (!params_) {
        return defaultValue;
    }
    return params_->getStringBool(parameterKey, defaultValue);
}

std::string LectorConfig::getPath() const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    return path_;
}

bool LectorConfig::isInitialized() const
{
    std::lock_guard<std::mutex> mutexGuard(mtx_);
    return initialized_;
}
