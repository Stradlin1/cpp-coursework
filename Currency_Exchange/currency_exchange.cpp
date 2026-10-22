#include <array>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <regex>
#include <fstream>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

using std::array;
using std::cin;
using std::cout;
using std::fixed;
using std::setprecision;
using std::size_t;
using std::string;
using std::vector;

namespace {

template <typename T>
bool readNumber(T& value) {
    std::string token;
    if (!(std::cin >> token)) return false;
    std::istringstream input(token);
    T parsed{};
    if (!(input >> parsed) || input.peek() != std::char_traits<char>::eof()) return false;
    value = parsed;
    return true;
}

struct Currency {
    string code;
    string name;
};

const vector<Currency> kCurrencies{
    {"GBP", "英镑"}, {"EUR", "欧元"}, {"USD", "美元"},
    {"JPY", "日元"}, {"HKD", "港币"}, {"AUD", "澳元"}};

bool readInteger(int& value) {
    if (readNumber(value)) {
        return true;
    }
    if (cin.eof()) {
        return false;
    }
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return false;
}

bool readNonNegativeAmount(double& amount) {
    if (!(readNumber(amount)) || !std::isfinite(amount) || amount < 0) {
        if (cin.eof()) return false;
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    return true;
}

struct RateResult {
    double rate = 0;
    string source;
};

class IRateProvider {
public:
    virtual ~IRateProvider() = default;
    virtual bool get(const Currency& currency, RateResult& result, string& error) = 0;
};

class OnlineRateProvider final : public IRateProvider {
public:
    bool get(const Currency& currency, RateResult& result, string& error) override {
        const string url = "https://open.er-api.com/v6/latest/" + currency.code;
        string response;
        const char* offline = std::getenv("CURRENCY_EXCHANGE_OFFLINE");
        if (offline != nullptr && string(offline) == "1") {
            error = "测试模式已禁用联网请求";
            return false;
        }
        const string command =
            "curl --fail --silent --show-error --location "
            "--connect-timeout 8 --max-time 12 "
            "--user-agent \"CurrencyExchangeHomework/1.0\" \"" +
            url + "\"";
#ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
#else
        FILE* pipe = popen(command.c_str(), "r");
#endif
        if (pipe == nullptr) {
            error = "无法启动 curl 网络工具";
            return false;
        }
        array<char, 512> buffer{};
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            response += buffer.data();
        }
#ifdef _WIN32
        const int exitCode = _pclose(pipe);
#else
        const int exitCode = pclose(pipe);
#endif
        if (exitCode != 0) {
            error = "curl 请求失败，请检查网络连接";
            return false;
        }

        const std::regex cnyPattern(R"("CNY"\s*:\s*(-?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)\s*[,}])");
        std::smatch match;
        if (!std::regex_search(response, match, cnyPattern) || match.size() < 2) {
            error = "返回内容中没有有效的 CNY 汇率";
            return false;
        }
        try {
            result.rate = std::stod(match[1].str());
        } catch (const std::invalid_argument&) {
            error = "汇率格式无法解析";
            return false;
        } catch (const std::out_of_range&) {
            error = "汇率超出可处理范围";
            return false;
        }
        if (!std::isfinite(result.rate) || result.rate <= 0) {
            error = "服务器返回了无效汇率";
            return false;
        }
        result.source = "在线 API";
        return true;
    }
};

class FileRateCache final : public IRateProvider {
public:
    explicit FileRateCache(string filename) : filename_(std::move(filename)) {}

    bool get(const Currency& currency, RateResult& result, string& error) override {
        std::ifstream input(filename_);
        string code;
        double rate = 0;
        long long timestamp = 0;
        string line;
        while (std::getline(input, line)) {
            std::istringstream record(line);
            if (!(record >> code >> rate >> timestamp) || !(record >> std::ws).eof()) continue;
            if (code == currency.code && std::isfinite(rate) && rate > 0) {
                const long long now = static_cast<long long>(std::time(nullptr));
                if (timestamp < 0 || timestamp > now || timestamp < now - kMaxAgeSeconds) {
                    error = currency.name + " 的本地缓存已过期";
                    return false;
                }
                result.rate = rate;
                result.source = "本地缓存（" + formatTime(timestamp) + "）";
                return true;
            }
        }
        error = "没有找到 " + currency.name + " 的本地缓存";
        return false;
    }

    void put(const Currency& currency, double rate) const {
        vector<string> lines;
        std::ifstream input(filename_);
        string code;
        double oldRate = 0;
        long long oldTimestamp = 0;
        string line;
        while (std::getline(input, line)) {
            std::istringstream record(line);
            if (!(record >> code >> oldRate >> oldTimestamp) || !(record >> std::ws).eof()) continue;
            if (code != currency.code) {
                lines.push_back(line);
            }
        }
        std::ostringstream newRecord;
        newRecord << setprecision(std::numeric_limits<double>::max_digits10)
                  << currency.code << ' ' << rate << ' '
                  << static_cast<long long>(std::time(nullptr));
        lines.push_back(newRecord.str());

        std::ofstream output(filename_, std::ios::trunc);
        if (!output) {
            std::cerr << "警告：无法写入汇率缓存文件 " << filename_ << '\n';
            return;
        }
        for (const string& line : lines) output << line << '\n';
    }

private:
    static constexpr long long kMaxAgeSeconds = 24 * 60 * 60;
    string filename_;

    static string formatTime(long long timestamp) {
        const std::time_t rawTime = static_cast<std::time_t>(timestamp);
        std::tm* localTime = std::localtime(&rawTime);
        if (localTime == nullptr) return "未知时间";
        char buffer[32]{};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
        return buffer;
    }
};

class RateService {
public:
    RateService() : cache_("exchange_rates.cache") {}

    bool get(const Currency& currency, RateResult& result, string& error) {
        string onlineError;
        if (online_.get(currency, result, onlineError)) {
            cache_.put(currency, result.rate);
            return true;
        }

        string cacheError;
        if (cache_.get(currency, result, cacheError)) {
            error = onlineError + "；已切换到" + result.source;
            return true;
        }

        error = onlineError + "；" + cacheError;
        return false;
    }

private:
    OnlineRateProvider online_;
    FileRateCache cache_;
};

class ExchangeApplication {
public:
    void run() {
        bool running = true;
        while (running) {
            printMainMenu();
            int choice = 0;
            if (!readInteger(choice)) {
                if (cin.eof()) return;
                cout << "输入无效，请输入菜单编号。\n";
                continue;
            }
            switch (choice) {
                case 1:
                    chooseCurrency(true);
                    break;
                case 2:
                    chooseCurrency(false);
                    break;
                case 3:
                    running = false;
                    break;
                default:
                    cout << "没有这个选项，请重新选择。\n";
            }
        }
        cout << "感谢使用外币兑换系统。\n";
    }

private:
    RateService rateService_;

    static void printMainMenu() {
        cout << "\n========== 外币兑换系统 ==========\n"
             << "1. 外币兑换人民币\n"
             << "2. 人民币兑换外币\n"
             << "3. 退出系统\n"
             << "请选择：";
    }

    void chooseCurrency(bool foreignToCny) {
        while (true) {
            cout << "\n" << (foreignToCny ? "外币兑换人民币" : "人民币兑换外币")
                 << "\n请选择币种：\n";
            for (size_t i = 0; i < kCurrencies.size(); ++i) {
                cout << i + 1 << ". " << kCurrencies[i].name << '\n';
            }
            cout << kCurrencies.size() + 1 << ". 返回上一级\n请选择：";

            int choice = 0;
            if (!readInteger(choice)) {
                if (cin.eof()) return;
                cout << "输入无效，请输入菜单编号。\n";
                continue;
            }
            if (choice == static_cast<int>(kCurrencies.size()) + 1) {
                return;
            }
            if (choice < 1 || choice > static_cast<int>(kCurrencies.size())) {
                cout << "没有这个选项。\n";
                continue;
            }
            exchange(foreignToCny, kCurrencies[choice - 1]);
        }
    }

    void exchange(bool foreignToCny, const Currency& currency) {
        double rate = 0;
        string error;
        cout << "汇率方式：1. 自动查询（失败时手动输入） 2. 手动输入：";
        int mode = 0;
        if (!readInteger(mode) || (mode != 1 && mode != 2)) {
            if (!cin.eof()) cout << "汇率方式无效，本次操作取消。\n";
            return;
        }
        if (mode == 1) cout << "正在查询 " << currency.name << " 对人民币的汇率...\n";
        RateResult rateResult;
        if (mode == 2 || !rateService_.get(currency, rateResult, error)) {
            if (mode == 1) cout << "联网查询失败：" << error << "\n";
            cout << "请手动输入 1 " << currency.name << " 兑换人民币的汇率：";
            if (!(readNumber(rate)) || !std::isfinite(rate) || rate <= 0) {
                if (cin.eof()) return;
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "汇率必须是大于 0 的数字，本次操作取消。\n";
                return;
            }
            rateResult.rate = rate;
            rateResult.source = "用户手动输入";
        }

        if (mode == 1 && !error.empty() && rateResult.source != "用户手动输入")
            cout << "联网查询失败：" << error << '\n';
        double amount = 0;
        cout << "查询到的参考汇率：1 " << currency.name << " = "
             << fixed << setprecision(6) << rateResult.rate << " 元\n"
             << "汇率来源：" << rateResult.source << '\n';
        cout << "请输入" << (foreignToCny ? currency.name : "人民币") << "金额：";
        if (!readNonNegativeAmount(amount)) {
            if (cin.eof()) return;
            cout << "金额必须是大于等于 0 的数字。\n";
            return;
        }

        const double result = foreignToCny ? amount * rateResult.rate : amount / rateResult.rate;
        if (!std::isfinite(result)) {
            cout << "兑换结果超出可表示范围，本次操作取消。\n";
            return;
        }
        cout << fixed << setprecision(2)
             << "兑换结果：" << result
             << (foreignToCny ? " 元人民币" : " " + currency.name) << "\n";
    }
};

}  // namespace

int main() {
    ExchangeApplication app;
    app.run();
    return 0;
}
