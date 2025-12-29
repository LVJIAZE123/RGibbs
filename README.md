# RGibbs

## 项目概述
RGibbs 提供一个简化的 CAPE-OPEN 单元操作（吉布斯自由能最小化反应器）示例，实现端口/参数暴露、生命周期管理以及热力学接口占位。代码包含中文注释，便于理解接口实现思路。

## 目录结构
- `src/`：核心 C++ 模块与接口定义。
- `tests/`：最小示例/单元测试。
- `CMakeLists.txt`：构建与测试配置。

## 构建与测试
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## 使用方法
1. 通过 `setThermoPackage` 注入实现 `ThermoPackage` 的热力学包。
2. 使用 `setFeed` 配置进料物料端口（组成、温度、压力）。
3. 调用生命周期方法：
   - `Initialize()`：检查热力学包等依赖。
   - `Validate()`：校验参数与物性。
   - `Calculate()`：执行 Gibbs 自由能最小化（占位迭代流程：化学势计算→拉格朗日乘子近似→更新组成→写回产品端口）。
   - `Terminate()`：释放状态。
4. 通过 `getProduct()` 读取产品端口状态（温度、压力、组成）。

### 接口说明
- 输入：进料 `MaterialPort`（`composition` 摩尔数映射、`temperature`、`pressure`），操作条件 `setTemperature/setPressure`。
- 输出：产品 `MaterialPort`（同构成字段，包含平衡后组成）。
- 热力学接口：`ThermoPackage::chemicalPotential`、`ThermoPackage::gibbsEnergy`。可在 `Calculate` 中根据吉布斯自由能最小化算法调用专业热力学求解器。
- 支持相态/元素约束：示例中为单相、守恒总摩尔数的占位实现；可扩展为气/液/固多相与元素守恒约束。

## 示例
参考 `tests/test_reactor.cpp`，其中提供了热力学桩件 `DummyThermo`，演示参数设置、正常计算与异常路径（未初始化、进料为空）。
