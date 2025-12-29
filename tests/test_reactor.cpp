/**
 * @file test_reactor.cpp
 * @brief 使用最小桩件测试 RGibbsReactor。
 */
#include "RGibbsReactor.h"

#include <cassert>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

using namespace capeopen;

// 简单的热力学桩件，假设化学势与摩尔分数线性相关
class DummyThermo : public ThermoPackage {
public:
    std::map<std::string, double> chemicalPotential(const MaterialPort& state) override {
        std::map<std::string, double> mu;
        double total = 0.0;
        for (const auto& kv : state.composition) total += kv.second;
        for (const auto& kv : state.composition) {
            double xi = (total > 0) ? kv.second / total : 0.0;
            mu[kv.first] = xi;  // 线性占位
        }
        return mu;
    }

    double gibbsEnergy(const MaterialPort& state) override {
        double total = 0.0;
        for (const auto& kv : state.composition) total += kv.second;
        return total * 1000.0;  // 线性标度占位
    }
};

void test_successful_calculation() {
    RGibbsReactor reactor;
    reactor.setThermoPackage(std::make_shared<DummyThermo>());
    MaterialPort feed{"Feed", {{"A", 1.0}, {"B", 2.0}}, 300.0, 101325.0};
    reactor.setFeed(feed);
    reactor.setTemperature(500.0);
    reactor.setPressure(2e5);

    reactor.Initialize();
    reactor.Calculate();
    auto product = reactor.getProduct();

    assert(std::abs(product.temperature - 500.0) < 1e-6);
    assert(std::abs(product.pressure - 2e5) < 1e-6);
    assert(product.composition["A"] > 0);
    assert(product.composition["B"] > 0);
    std::cout << "test_successful_calculation passed" << std::endl;
}

void test_validation_failure() {
    RGibbsReactor reactor;
    reactor.setThermoPackage(std::make_shared<DummyThermo>());
    MaterialPort feed{"Feed", {}, 300.0, 101325.0};
    reactor.setFeed(feed);
    reactor.Initialize();
    bool threw = false;
    try {
        reactor.Calculate();
    } catch (const CapeOpenException& ex) {
        threw = (ex.code() == CapeOpenError::CapeInvalidArgument);
    }
    assert(threw && "应当因为空组成而失败");
    std::cout << "test_validation_failure passed" << std::endl;
}

void test_operation_without_init() {
    RGibbsReactor reactor;
    bool threw = false;
    try {
        reactor.Calculate();
    } catch (const CapeOpenException& ex) {
        threw = (ex.code() == CapeOpenError::CapeInvalidOperation);
    }
    assert(threw && "未初始化时应抛出异常");
    std::cout << "test_operation_without_init passed" << std::endl;
}

int main() {
    test_successful_calculation();
    test_validation_failure();
    test_operation_without_init();
    std::cout << "所有测试通过" << std::endl;
    return 0;
}
