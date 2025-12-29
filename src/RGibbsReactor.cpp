/**
 * @file RGibbsReactor.cpp
 * @brief CAPE-OPEN 吉布斯自由能反应器占位实现。
 */
#include "RGibbsReactor.h"

#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>

namespace capeopen {

RGibbsReactor::RGibbsReactor() { log("RGibbsReactor 构造完成"); }

void RGibbsReactor::Initialize() {
    if (!thermo_) {
        throw CapeOpenException(CapeOpenError::CapeFailedInitialization, "未配置热力学包");
    }
    initialized_ = true;
    calculated_ = false;
    log("初始化成功");
}

void RGibbsReactor::Validate() {
    ensureInitialized();
    if (feed_.composition.empty()) {
        throw CapeOpenException(CapeOpenError::CapeInvalidArgument, "进料组成为空");
    }
    if (temperature_ <= 0 || pressure_ <= 0) {
        throw CapeOpenException(CapeOpenError::CapeInvalidArgument, "温度或压力无效");
    }
    log("验证通过");
}

void RGibbsReactor::Calculate() {
    ensureInitialized();
    try {
        Validate();
        performGibbsMinimization();
        calculated_ = true;
        log("计算完成");
    } catch (const CapeOpenException&) {
        throw;
    } catch (const std::exception& ex) {
        throw CapeOpenException(CapeOpenError::CapeCalculationFailed, ex.what());
    }
}

void RGibbsReactor::Terminate() {
    initialized_ = false;
    calculated_ = false;
    log("终止完成，资源已释放");
}

void RGibbsReactor::setFeed(const MaterialPort& feed) { feed_ = feed; }

void RGibbsReactor::setThermoPackage(std::shared_ptr<ThermoPackage> pkg) {
    thermo_ = std::move(pkg);
}

void RGibbsReactor::setTemperature(double t) { temperature_ = t; }

void RGibbsReactor::setPressure(double p) { pressure_ = p; }

MaterialPort RGibbsReactor::getProduct() const {
    if (!calculated_) {
        throw CapeOpenException(CapeOpenError::CapeInvalidOperation, "尚未完成计算");
    }
    return product_;
}

void RGibbsReactor::log(const std::string& message) const {
    std::cerr << "[RGibbsReactor] " << message << std::endl;
}

void RGibbsReactor::ensureInitialized() const {
    if (!initialized_) {
        throw CapeOpenException(CapeOpenError::CapeInvalidOperation, "请先调用 Initialize");
    }
}

void RGibbsReactor::performGibbsMinimization() {
    // 复制进料到产品，初始化状态
    product_ = feed_;
    product_.temperature = temperature_;
    product_.pressure = pressure_;

    // 简化实现：利用化学势迭代调整组成直至收敛（占位）
    auto mu = thermo_->chemicalPotential(product_);
    double totalMoles = 0.0;
    for (const auto& kv : product_.composition) {
        totalMoles += kv.second;
    }
    if (totalMoles <= 0) {
        throw CapeOpenException(CapeOpenError::CapeCalculationFailed, "物料摩尔数无效");
    }

    // 假设通过 Lagrange 乘子 λ，调整每个组分 n_i = max(n_i - alpha*(mu_i - λ), 0)
    // 这里 alpha 为简化步长，λ 取平均化学势，迭代若干步
    const double alpha = 0.1;
    for (int iter = 0; iter < 20; ++iter) {
        double lambda = 0.0;
        for (const auto& kv : mu) {
            lambda += kv.second;
        }
        lambda /= static_cast<double>(mu.size());
        for (auto& kv : product_.composition) {
            const auto it = mu.find(kv.first);
            if (it == mu.end()) continue;
            double delta = alpha * (it->second - lambda);
            kv.second = std::max(0.0, kv.second - delta);
        }
        // 重新归一化保持总摩尔数
        double newTotal = 0.0;
        for (const auto& kv : product_.composition) newTotal += kv.second;
        if (newTotal <= 0) {
            throw CapeOpenException(CapeOpenError::CapeCalculationFailed, "迭代得到无效组成");
        }
        for (auto& kv : product_.composition) {
            kv.second *= totalMoles / newTotal;
        }
        mu = thermo_->chemicalPotential(product_);
    }

    double g = thermo_->gibbsEnergy(product_);
    log("迭代完成，Gibbs=" + std::to_string(g));
}

}  // namespace capeopen
