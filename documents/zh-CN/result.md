# cepo::result<T, E>

1. 定位

cepo::result<T, E> 是一个 C++11 的 move-only 结果类型，用于把"值 / 错误值 / 构造异常"三种可能收敛成单一对象。设计受 Rust Result 启发，但在异常处理上与 Rust 不同：C++ 的 T/E 构造或移动过程中可能抛异常，result 会捕获并存储，而不是让异常逃逸。

核心保证：

· 无默认构造、无拷贝构造
· T/E 的构造与移动异常在 result 或包装类型的构造函数中捕获
· 所有移动操作 noexcept（异常不逃逸，但可能被捕获存储）

---

2. 类型组成

类型 角色 公开接口
result<T, E> 终态对象 见 §3–§5
result_value_type<T> 值的构造载体 移动构造/赋值、默认构造（SFINAE）、emplace-like（SFINAE，explicit）
result_error_type<E> 错误的构造载体 同上

· result 是后两者的 friend
· 后两者只用于 make_result_* → result 的传递，不承担状态语义
· 包装类型的构造均 explicit（除移动构造），均 noexcept（内部捕获）

---

3. 状态机（核心）

四个查询接口：is_ok() / is_error() / is_empty() / has_exception()。

四种合法状态（推导表闭合，16 种组合收敛到 4 种）：

状态 is_ok is_error is_empty has_exception 成因
Value ✅ ❌ ❌ ❌ 持有 T
Error ❌ ✅ ❌ ❌ 持有 E
Failed ❌ ❌ ✅ ✅ T/E 构造或移动时抛异常
Moved-from ❌ ❌ ✅ ❌ result 已被移动

推导关系：

```
has_exception          ⇒ is_empty
is_empty               ⇔ !is_ok && !is_error
is_error               ⇒ !is_ok && !has_exception
is_ok                  ⇒ !is_error && !has_exception
is_moved（无此函数）    ⇔ is_empty && !has_exception
构造或移动失败          ⇔ is_empty && has_exception
```

异常状态移动时：源变为 Moved-from，目标继承 Failed 状态（exception_ptr 转移）。

---

4. 状态查询

```cpp
bool is_ok() const noexcept;
bool is_error() const noexcept;
bool is_empty() const noexcept;      // == !is_ok && !is_error
bool has_exception() const noexcept;
std::exception_ptr get_exception() const noexcept;  // 无异常时返回空
```

⚠️ has_exception() 在极端内存耗尽时可能返回 false（std::current_exception() 返回空 exception_ptr）。这是标准库边界，本库不对此负责。

---

5. 解包

```cpp
T&       unwrap_value();              // T=void 时不存在（两个重载均不存在）
const T& unwrap_value() const;
const E& unwrap_error() const;
```

· 状态不匹配（Value ↔ Error 方向、或 any Empty）→ 抛 cepo::bad_result_access
· bad_result_access 不携带原始异常，只表示"获取失败"。需要原始异常请先 has_exception() + get_exception()
· 错误值始终只读：修改错误会掩盖原始信息

---

6. 消费：ok(F) → error(F) / ignore_error()

```cpp
res.ok([](T& v) { /* 可修改 v 的状态 */ })
   .error([](const E& e) { /* 只读 */ });

// 或忽略错误
res.ok([](T& v) { ... }).ignore_error();
```

· ok(F) 返回 when_error（保存 result*，禁拷贝）
· when_error::error(F) && / when_error::ignore_error() && 是终态操作，返回 void
· 回调参数不能为空（T=void 时 ok 无参）
· F 必须是 void 返回值
· 库不捕获 F 的异常：抛出的异常会向调用方传播，result 状态不变（F 对 T 的部分修改不回滚）
· 状态为空时静默不处理——异常路径必须用 has_exception() 另行检查

严格禁止：

· 保存 when_error（成员、容器、返回值、auto&& 都不行）
· 唯一合法用法是立即链式消费

---

7. 构造

```cpp
// 直接构造（三种右值构造）
result<T, E> r = std::move(other_result);
result<T, E> r = make_result_value<T>(args...);
result<T, E> r = make_result_error<E>(args...);

// 工厂
auto v = make_result_value<T>(args...) noexcept;   // T=void 时 0 参数
auto e = make_result_error<E>(args...) noexcept;
```

· 工厂不负责捕获——异常已在包装类型构造时被捕获
· 工厂的 noexcept 表示"不逃逸异常"，不代表 T/E 构造/移动成功
· 等价关系：make_result_value<T>(args...) ≡ result_value_type<T>(args...)

---

8. 移动语义

所有移动操作 noexcept。含义：异常不逃逸，但 T/E 移动过程中可能抛异常，抛出的异常会被捕获并存储。

源状态 目标状态 源状态
Value（移动成功） Value Moved-from
Value（T 移动抛异常） Failed（捕获异常） Moved-from
Error（E 移动抛异常） Failed（捕获异常） Moved-from
Failed Failed（继承异常） Moved-from
Moved-from Moved-from Moved-from

移动赋值的额外步骤：先 destroy_self()（析构当前持有的 T/E），再接管源状态。

⚠️ 用户理解要点：noexcept 移动 ≠ 移动一定成功。std::vector<result>::reserve 走移动分支时，若 T 的移动构造抛异常，元素会静默变成 Failed。

---

9. 类型约束（static_assert）

```cpp
static_assert(!std::is_void<E>::value);                        // E=void 禁止
static_assert(std::is_nothrow_destructible<T>::value);          // T 析构 noexcept
static_assert(std::is_nothrow_destructible<E>::value);          // E 析构 noexcept
```

T=void 合法（此时 unwrap_value 的两个重载都不存在，ok(F) 的参数也为空）。

---

10. 已知边界

· 极端内存耗尽：has_exception() 可能返回 false 但实际有异常发生；is_empty && !has_exception 此时对应"异常捕获失败"而非"已移动"
· const result&：不支持 ok/error 链（F 可能修改 T）；如需只读，用 unwrap_value() const
· self-move-assignment：r = std::move(r) 后 r 为 Moved-from（有效但未指定）
