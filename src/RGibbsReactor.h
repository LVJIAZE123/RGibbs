/**
 * @file RGibbsReactor.h
 * @brief 定义满足 CAPE-OPEN 单元操作接口的吉布斯自由能反应器占位实现。
 */
#pragma once

#include <exception>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace capeopen {

// 简化的 CAPE-OPEN 错误码定义（示例）
enum class CapeOpenError {
    CapeNoError = 0,
    CapeUnknownError = 1,
    CapeInvalidArgument = 2,
    CapeInvalidOperation = 3,
    CapeFailedInitialization = 4,
    CapeCalculationFailed = 5,
};

// 自定义异常以符合 CAPE-OPEN 风格
class CapeOpenException : public std::exception {
public:
    CapeOpenException(CapeOpenError code, std::string message)
        : code_(code), message_(std::move(message)) {}

    const char* what() const noexcept override { return message_.c_str(); }
    CapeOpenError code() const noexcept { return code_; }

private:
    CapeOpenError code_;
    std::string message_;
};

/**
 * @brief 物料端口，封装相态、组成与热力学状态。
 */
struct MaterialPort {
    std::string name;
    // 物种摩尔数
    std::map<std::string, double> composition;
    // 温度、压力
    double temperature{298.15};
    double pressure{101325.0};
};

/**
 * @brief 热力学包接口占位，提供必要的计算入口。
 */
class ThermoPackage {
public:
    virtual ~ThermoPackage() = default;
    // 计算给定组成/状态的化学势
    virtual std::map<std::string, double> chemicalPotential(const MaterialPort& state) = 0;
    // 计算 Gibbs 自由能
    virtual double gibbsEnergy(const MaterialPort& state) = 0;
};

/**
 * @brief 吉布斯自由能最小化反应器占位实现。
 */
class RGibbsReactor {
public:
    RGibbsReactor();

    // 生命周期管理
    void Initialize();
    void Validate();
    void Calculate();
    void Terminate();

    // 端口与参数配置
    void setFeed(const MaterialPort& feed);
    void setThermoPackage(std::shared_ptr<ThermoPackage> pkg);
    void setTemperature(double t);
    void setPressure(double p);
    MaterialPort getProduct() const;

private:
    void log(const std::string& message) const;
    void ensureInitialized() const;
    void performGibbsMinimization();

    bool initialized_{false};
    bool calculated_{false};
    MaterialPort feed_;
    MaterialPort product_;
    double temperature_{298.15};
    double pressure_{101325.0};
    std::shared_ptr<ThermoPackage> thermo_;
};

}  // namespace capeopen
