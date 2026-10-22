#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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

enum class Role { GeneralManager = 1, TechnicalManager, Technician, SalesManager, Salesperson };
enum class UserRole { Administrator, Viewer };

int displayWidth(const std::string& text) {
    int width = 0;
    for (size_t i = 0; i < text.size();) {
        const unsigned char byte = static_cast<unsigned char>(text[i]);
        if (byte < 0x80) {
            ++width;
            ++i;
        } else if (byte < 0xE0 && i + 1 < text.size()) {
            width += 2;
            i += 2;
        } else if (byte < 0xF0 && i + 2 < text.size()) {
            width += 2;
            i += 3;
        } else if (i + 3 < text.size()) {
            width += 2;
            i += 4;
        } else {
            ++width;
            ++i;
        }
    }
    return width;
}

void printCell(const std::string& text, int width) {
    std::cout << text;
    const int padding = width - displayWidth(text);
    if (padding > 0) std::cout << std::string(static_cast<size_t>(padding), ' ');
}

double readMoney(const char* prompt) {
    while (true) {
        std::cout << prompt;
        double value = 0;
        if (readNumber(value) && std::isfinite(value) && value >= 0) return value;
        if (std::cin.eof()) throw std::runtime_error("输入结束，未完成的操作已取消");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "请输入非负数字。\n";
    }
}

class Employee {
public:
    explicit Employee(std::string name) : name_(std::move(name)) {}
    virtual ~Employee() = default;
    virtual std::string role() const = 0;
    int level() const { return level_; }
    void promote() { level_ = targetLevel(); }
    virtual int targetLevel() const = 0;
    virtual double salary(double departmentSales) const = 0;
    virtual Role roleCode() const = 0;
    virtual double extraValue() const { return 0; }
    virtual void setExtraValue(double) {}

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    void setId(int id) { id_ = id; }
    virtual void displayExtra() const {}
    virtual void editExtra() = 0;

protected:
    std::string name_;
    int id_ = 0;
    int level_ = 1;  // 所有新对象从 1 级开始，由升级操作设置课程指定级别。
};

class FixedEmployee : public Employee {
public:
    using Employee::Employee;
    void editExtra() override {}
};

class BonusEmployee : public Employee {
public:
    BonusEmployee(std::string name, double base) : Employee(std::move(name)), base_(base) {}
    double salary(double) const override { return base_ + bonus_; }
    void displayExtra() const override { std::cout << "奖金=" << bonus_; }
    void editExtra() override { bonus_ = readMoney("请输入奖金："); }
    double extraValue() const override { return bonus_; }
    void setExtraValue(double value) override { bonus_ = value; }

protected:
    double base_;
    double bonus_ = 0;
};

class SalesEmployee : public Employee {
public:
    SalesEmployee(std::string name, double base, double rate)
        : Employee(std::move(name)), base_(base), rate_(rate) {}
    double salary(double) const override { return base_ + sales_ * rate_; }
    void displayExtra() const override { std::cout << "销售额=" << sales_; }
    void editExtra() override { sales_ = readMoney("请输入当月销售额："); }
    double sales() const { return sales_; }
    double extraValue() const override { return sales_; }
    void setExtraValue(double value) override { sales_ = value; }

protected:
    double base_;
    double rate_;
    double sales_ = 0;
};

class GeneralManager final : public FixedEmployee {
public:
    using FixedEmployee::FixedEmployee;
    std::string role() const override { return "总经理"; }
    int targetLevel() const override { return 4; }
    double salary(double) const override { return 20000; }
    Role roleCode() const override { return Role::GeneralManager; }
};

class TechnicalManager final : public BonusEmployee {
public:
    explicit TechnicalManager(std::string name) : BonusEmployee(std::move(name), 12000) {}
    std::string role() const override { return "技术经理"; }
    int targetLevel() const override { return 3; }
    Role roleCode() const override { return Role::TechnicalManager; }
};

class Technician final : public BonusEmployee {
public:
    explicit Technician(std::string name) : BonusEmployee(std::move(name), 6000) {}
    std::string role() const override { return "技术人员"; }
    int targetLevel() const override { return 2; }
    Role roleCode() const override { return Role::Technician; }
};

class SalesManager final : public Employee {
public:
    using Employee::Employee;
    std::string role() const override { return "销售经理"; }
    int targetLevel() const override { return 3; }
    double salary(double departmentSales) const override { return 8000 + departmentSales * 0.005; }
    void editExtra() override {}
    Role roleCode() const override { return Role::SalesManager; }
};

class Salesperson final : public SalesEmployee {
public:
    explicit Salesperson(std::string name) : SalesEmployee(std::move(name), 0, 0.01) {}
    std::string role() const override { return "销售人员"; }
    int targetLevel() const override { return 1; }
    Role roleCode() const override { return Role::Salesperson; }
};

class EmployeeFactory {
public:
    static std::unique_ptr<Employee> create(Role role, const std::string& name) {
        std::unique_ptr<Employee> employee;
        switch (role) {
            case Role::GeneralManager: employee = std::make_unique<GeneralManager>(name); break;
            case Role::TechnicalManager: employee = std::make_unique<TechnicalManager>(name); break;
            case Role::Technician: employee = std::make_unique<Technician>(name); break;
            case Role::SalesManager: employee = std::make_unique<SalesManager>(name); break;
            case Role::Salesperson: employee = std::make_unique<Salesperson>(name); break;
        }
        if (employee) employee->promote();
        return employee;
    }
};

class EmployeeSystem {
public:
    explicit EmployeeSystem(const std::filesystem::path& executablePath)
        : storageFile_(executablePath.parent_path() / "employees.dat"),
          logFile_(executablePath.parent_path() / "employee_operations.log") {
        load();
    }
    ~EmployeeSystem() { if (dirty_) save(); }

    void run() {
        if (!login()) return;
        int choice = 0;
        while (true) {
            std::cout << "\n========== 公司职员管理系统 ==========\n"
                      << "当前用户：" << username_ << "（"
                      << (userRole_ == UserRole::Administrator ? "管理员" : "普通用户") << "）\n"
                      << "1. 显示全部\n2. 查询员工\n3. 计算月薪总额\n";
            if (userRole_ == UserRole::Administrator) {
                std::cout << "4. 录入员工\n5. 修改员工\n6. 删除员工\n7. 查看操作日志\n";
            }
            std::cout << "0. 退出\n请选择：";
            if (!(readNumber(choice))) {
                if (std::cin.eof()) break;
                clearInput();
                std::cout << "请输入数字。\n";
                continue;
            }
            switch (choice) {
                case 1: list(); break;
                case 2: search(); break;
                case 3: totalSalary(); break;
                case 4: if (isAdministrator()) add(); else deny(); break;
                case 5: if (isAdministrator()) edit(); else deny(); break;
                case 6: if (isAdministrator()) remove(); else deny(); break;
                case 7: if (isAdministrator()) showLog(); else deny(); break;
                case 0: return;
                default: std::cout << "没有这个选项。\n";
            }
        }
    }

private:
    std::vector<std::unique_ptr<Employee>> employees_;
    std::filesystem::path storageFile_;
    std::filesystem::path logFile_;
    bool dirty_ = false;
    std::string username_;
    UserRole userRole_ = UserRole::Viewer;

    static void clearInput() {
        if (std::cin.eof()) throw std::runtime_error("输入结束，未完成的操作已取消");
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    static std::string currentTime() {
        const std::time_t now = std::time(nullptr);
        std::tm localTime{};
        if (const std::tm* value = std::localtime(&now); value != nullptr) localTime = *value;
        std::ostringstream output;
        output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return output.str();
    }

    bool isAdministrator() const {
        return userRole_ == UserRole::Administrator;
    }

    void logOperation(const std::string& action) const {
        std::ofstream output(logFile_, std::ios::app);
        if (!output) {
            std::cerr << "警告：无法写入操作日志 " << logFile_.string() << "\n";
            return;
        }
        output << currentTime() << " | 用户=" << username_ << " | " << action << '\n';
    }

    bool login() {
        for (int attempt = 1; attempt <= 3; ++attempt) {
            std::string username;
            std::string password;
            std::cout << "\n========== 用户登录 ==========\n"
                      << "用户名：";
            if (!std::getline(std::cin >> std::ws, username)) return false;
            std::cout << "密码：";
            if (!std::getline(std::cin, password)) return false;

            if (username == "admin" && password == "admin") {
                username_ = username;
                userRole_ = UserRole::Administrator;
                logOperation("登录成功（管理员）");
                std::cout << "登录成功，欢迎管理员。\n";
                return true;
            }
            if (username == "user" && password == "user") {
                username_ = username;
                userRole_ = UserRole::Viewer;
                logOperation("登录成功（普通用户）");
                std::cout << "登录成功，普通用户只能查看和查询数据。\n";
                return true;
            }
            std::cout << "用户名或密码错误，还可尝试 " << 3 - attempt << " 次。\n";
            username_ = username;
            logOperation("登录失败");
        }
        std::cout << "登录失败次数过多，程序退出。\n";
        return false;
    }

    void deny() const {
        std::cout << "权限不足：当前账号只能查看、查询和统计数据。\n";
    }

    static std::string readName() {
        std::string name;
        std::cout << "请输入姓名：";
        std::getline(std::cin >> std::ws, name);
        return name;
    }

    double departmentSales() const {
        const double sales = std::accumulate(employees_.begin(), employees_.end(), 0.0,
            [](double total, const std::unique_ptr<Employee>& employee) {
                return total + (employee->roleCode() == Role::Salesperson
                                    ? employee->extraValue() : 0.0);
            });
        if (!std::isfinite(sales)) throw std::runtime_error("部门销售额超出可表示范围");
        return sales;
    }

    void load() {
        std::ifstream input(storageFile_);
        std::string line;
        while (std::getline(input, line)) {
            std::istringstream record(line);
            int role = 0;
            std::string name;
            double extra = 0;
            if (!(record >> role >> std::quoted(name) >> extra) ||
                !(record >> std::ws).eof() || name.find_first_not_of(" \t\r") == std::string::npos ||
                role < 1 || role > 5 || !std::isfinite(extra) || extra < 0) {
                std::cerr << "警告：跳过无效员工记录。\n";
                continue;
            }
            if (std::any_of(employees_.begin(), employees_.end(),
                    [&name](const auto& employee) { return employee->name() == name; })) {
                std::cerr << "警告：跳过重名员工记录。\n";
                continue;
            }
            auto employee = EmployeeFactory::create(static_cast<Role>(role), name);
            employee->setExtraValue(extra);
            employees_.push_back(std::move(employee));
        }
        reindex();
    }

    void save() const {
        std::ofstream output(storageFile_, std::ios::trunc);
        if (!output) {
            std::cerr << "警告：无法保存员工数据到 " << storageFile_.string() << "\n";
            return;
        }
        output << std::setprecision(17);
        for (const auto& employee : employees_) {
            output << static_cast<int>(employee->roleCode()) << ' '
                   << std::quoted(employee->name()) << ' '
                   << employee->extraValue() << '\n';
        }
    }

    void reindex() {
        for (size_t i = 0; i < employees_.size(); ++i) employees_[i]->setId(static_cast<int>(i + 1));
    }

    Employee* find(int id) const {
        for (const auto& employee : employees_) {
            if (employee->id() == id) return employee.get();
        }
        return nullptr;
    }

    static bool matchesRole(const Employee& employee, int role) {
        return static_cast<int>(employee.roleCode()) == role;
    }

    void print(const Employee& employee) const {
        printCell(std::to_string(employee.id()), 8);
        printCell(employee.name(), 14);
        printCell(employee.role(), 16);
        printCell(std::to_string(employee.level()), 8);
        std::ostringstream salary;
        salary << std::fixed << std::setprecision(2) << employee.salary(departmentSales());
        printCell(salary.str(), 14);
        employee.displayExtra();
        std::cout << '\n';
    }

    void printHeader() const {
        printCell("编号", 8);
        printCell("姓名", 14);
        printCell("职位", 16);
        printCell("级别", 8);
        printCell("月薪", 14);
        std::cout << "附加数据\n";
    }

    void add() {
        std::cout << "1.总经理 2.技术经理 3.技术人员 4.销售经理 5.销售人员\n请选择职位：";
        int choice = 0;
        if (!(readNumber(choice)) || choice < 1 || choice > 5) {
            clearInput();
            std::cout << "职位选择无效。\n";
            return;
        }
        const std::string name = readName();
        if (name.empty()) {
            std::cout << "姓名不能为空。\n";
            return;
        }
        for (const auto& employee : employees_) {
            if (employee->name() == name) {
                std::cout << "姓名已存在。\n";
                return;
            }
        }
        auto employee = EmployeeFactory::create(static_cast<Role>(choice), name);
        employee->editExtra();
        employees_.push_back(std::move(employee));
        reindex();
        dirty_ = true;
        logOperation("新增员工 | 编号=" + std::to_string(employees_.back()->id())
                     + " | 姓名=" + employees_.back()->name());
        std::cout << "录入成功。\n";
    }

    void list() const {
        if (employees_.empty()) { std::cout << "暂无员工。\n"; return; }
        printHeader();
        for (const auto& employee : employees_) print(*employee);
    }

    void search() const {
        std::cout << "\n----- 查询员工 -----\n"
                  << "1. 按编号查询\n"
                  << "2. 按姓名查询\n"
                  << "3. 按职位查询\n"
                  << "4. 按级别查询\n"
                  << "0. 返回上级\n"
                  << "请选择：";
        int choice = 0;
        if (!(readNumber(choice))) {
            clearInput();
            std::cout << "查询方式无效。\n";
            return;
        }
        if (choice == 0) return;

        std::vector<const Employee*> matches;
        if (choice == 1) {
            std::cout << "请输入员工编号：";
            int id = 0;
            if (!(readNumber(id))) {
                clearInput();
                std::cout << "编号无效。\n";
                return;
            }
            const Employee* employee = find(id);
            if (employee != nullptr) matches.push_back(employee);
        } else if (choice == 2) {
            std::string name;
            std::cout << "请输入员工姓名：";
            std::getline(std::cin >> std::ws, name);
            for (const auto& employee : employees_) {
                if (employee->name() == name) matches.push_back(employee.get());
            }
        } else if (choice == 3) {
            std::cout << "1.总经理 2.技术经理 3.技术人员 4.销售经理 5.销售人员\n"
                      << "请选择职位：";
            int role = 0;
            if (!(readNumber(role)) || role < 1 || role > 5) {
                clearInput();
                std::cout << "职位无效。\n";
                return;
            }
            for (const auto& employee : employees_) {
                if (matchesRole(*employee, role)) matches.push_back(employee.get());
            }
        } else if (choice == 4) {
            std::cout << "请输入级别（1-4）：";
            int level = 0;
            if (!(readNumber(level)) || level < 1 || level > 4) {
                clearInput();
                std::cout << "级别无效。\n";
                return;
            }
            for (const auto& employee : employees_) {
                if (employee->level() == level) matches.push_back(employee.get());
            }
        } else {
            std::cout << "查询方式无效。\n";
            return;
        }

        if (matches.empty()) {
            std::cout << "未找到匹配员工。\n";
            return;
        }
        printHeader();
        for (const Employee* employee : matches) print(*employee);
    }

    void edit() {
        std::cout << "请输入要修改的员工编号：";
        int id = 0;
        if (!(readNumber(id))) { clearInput(); std::cout << "编号无效。\n"; return; }
        Employee* employee = find(id);
        if (employee == nullptr) { std::cout << "未找到员工。\n"; return; }
        employee->editExtra();
        dirty_ = true;
        logOperation("修改员工 | 编号=" + std::to_string(id)
                     + " | 姓名=" + employee->name());
        std::cout << "修改成功。\n";
    }

    void remove() {
        std::cout << "请输入要删除的员工编号：";
        int id = 0;
        if (!(readNumber(id))) { clearInput(); std::cout << "编号无效。\n"; return; }
        const auto it = std::find_if(employees_.begin(), employees_.end(),
            [id](const auto& employee) { return employee->id() == id; });
        if (it == employees_.end()) { std::cout << "未找到员工。\n"; return; }
        const std::string name = (*it)->name();
        employees_.erase(it);
        reindex();
        dirty_ = true;
        logOperation("删除员工 | 编号=" + std::to_string(id) + " | 姓名=" + name);
        std::cout << "删除成功。\n";
    }

    void showLog() const {
        std::ifstream input(logFile_);
        if (!input) {
            std::cout << "暂无操作日志。\n";
            return;
        }
        std::cout << "\n----- 操作日志 -----\n";
        std::string line;
        while (std::getline(input, line)) std::cout << line << '\n';
    }

    void totalSalary() const {
        const double sales = departmentSales();
        const double total = std::accumulate(employees_.begin(), employees_.end(), 0.0,
            [sales](double sum, const auto& employee) { return sum + employee->salary(sales); });
        if (!std::isfinite(total)) throw std::runtime_error("工资总额超出可表示范围");
        std::cout << std::fixed << std::setprecision(2) << "当月薪资总额：" << total << " 元\n";
    }
};

int main(int argc, char* argv[]) {
    const std::filesystem::path executablePath =
        std::filesystem::absolute(
            argc > 0 ? std::filesystem::path(argv[0])
                     : std::filesystem::current_path() / "employee_system")
            ;
    EmployeeSystem system(executablePath);
    try { system.run(); }
    catch (const std::runtime_error& error) { std::cout << error.what() << '\n'; }
    return 0;
}
