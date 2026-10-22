# 外币兑换系统

这是大作业第一题的独立实现，支持：

- 外币兑换人民币、人民币兑换外币；
- 英镑、欧元、美元、日元、港币、澳元；
- 通过 `open.er-api.com` 联网查询汇率；
- 使用 `IRateProvider` 抽象汇率来源，在线查询和文件缓存可独立替换；
- 联网成功后保存到 `exchange_rates.cache`，网络不可用时优先读取本地缓存；
- 本地缓存默认有效期为 24 小时；过期缓存必须由用户明确确认后才能作为参考汇率使用；
- 在线查询、本地缓存、用户手动输入三级降级；
- 非法菜单、负数金额和无效汇率的检查。

## 编译与运行

Linux 环境需要安装 `g++` 和命令行 `curl`：

```bash
g++ -std=c++17 -Wall -Wextra -pedantic currency_exchange.cpp -o currency_exchange
./currency_exchange
```

程序通过 `curl` 命令访问公开汇率接口，不依赖 libcurl 开发头文件。联网查询失败时，程序会明确提示错误并允许手动输入参考汇率。

## 缓存测试

正常联网运行一次后会生成 `exchange_rates.cache`。可以使用下面的测试环境变量模拟断网，
验证程序从缓存恢复：

```bash
CURRENCY_EXCHANGE_OFFLINE=1 ./currency_exchange
```

如果缓存中没有对应币种，程序会提示手动输入汇率。

过期缓存交互回归测试：`python3 Currency_Exchange/tests/test_expired_cache.py`。
