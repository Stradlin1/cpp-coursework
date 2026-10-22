#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class UnsignedBigInteger {
public:
    explicit UnsignedBigInteger(std::string digits = "0") : digits_(std::move(digits)) {
        if (digits_.empty() ||
            !std::all_of(digits_.begin(), digits_.end(),
                         [](unsigned char c) { return std::isdigit(c) != 0; })) {
            throw std::invalid_argument("无符号整数只能包含数字");
        }
        normalize();
    }

    bool isZero() const { return digits_ == "0"; }

    int compare(const UnsignedBigInteger& other) const {
        if (digits_.size() != other.digits_.size()) {
            return digits_.size() < other.digits_.size() ? -1 : 1;
        }
        if (digits_ == other.digits_) return 0;
        return digits_ < other.digits_ ? -1 : 1;
    }

    UnsignedBigInteger operator+(const UnsignedBigInteger& other) const {
        std::string result;
        int carry = 0;
        int i = static_cast<int>(digits_.size()) - 1;
        int j = static_cast<int>(other.digits_.size()) - 1;
        while (i >= 0 || j >= 0 || carry != 0) {
            const int sum = carry + (i >= 0 ? digits_[i--] - '0' : 0) +
                            (j >= 0 ? other.digits_[j--] - '0' : 0);
            result.push_back(static_cast<char>('0' + sum % 10));
            carry = sum / 10;
        }
        std::reverse(result.begin(), result.end());
        return UnsignedBigInteger(result);
    }

    // 只计算较大数减较小数，负号由界面层负责显示。
    UnsignedBigInteger subtract(const UnsignedBigInteger& other) const {
        if (compare(other) < 0) {
            throw std::logic_error("减法结果不能为负");
        }
        std::string result;
        int borrow = 0;
        int i = static_cast<int>(digits_.size()) - 1;
        int j = static_cast<int>(other.digits_.size()) - 1;
        while (i >= 0) {
            int value = digits_[i--] - '0' - borrow -
                        (j >= 0 ? other.digits_[j--] - '0' : 0);
            if (value < 0) {
                value += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            result.push_back(static_cast<char>('0' + value));
        }
        std::reverse(result.begin(), result.end());
        return UnsignedBigInteger(result);
    }

    UnsignedBigInteger operator-(const UnsignedBigInteger& other) const {
        return subtract(other);
    }

    UnsignedBigInteger operator*(const UnsignedBigInteger& other) const {
        if (isZero() || other.isZero()) return UnsignedBigInteger("0");
        std::vector<int> digits(digits_.size() + other.digits_.size(), 0);
        for (int i = static_cast<int>(digits_.size()) - 1; i >= 0; --i) {
            for (int j = static_cast<int>(other.digits_.size()) - 1; j >= 0; --j) {
                const int position = i + j + 1;
                const int product = (digits_[i] - '0') * (other.digits_[j] - '0');
                digits[position] += product;
            }
        }
        for (int i = static_cast<int>(digits.size()) - 1; i > 0; --i) {
            digits[i - 1] += digits[i] / 10;
            digits[i] %= 10;
        }
        std::string result;
        size_t first = 0;
        while (first < digits.size() - 1 && digits[first] == 0) ++first;
        for (; first < digits.size(); ++first) {
            result.push_back(static_cast<char>('0' + digits[first]));
        }
        return UnsignedBigInteger(result);
    }

    UnsignedBigInteger operator/(const UnsignedBigInteger& divisor) const {
        return UnsignedBigInteger(divideStrings(*this, divisor).first);
    }

    UnsignedBigInteger operator%(const UnsignedBigInteger& divisor) const {
        return UnsignedBigInteger(divideStrings(*this, divisor).second);
    }

    // 科学计数法保留两位有效数字，整数商和余数由同一套长除法保证正确。
    std::string scientificQuotient(const UnsignedBigInteger& divisor) const {
        if (divisor.isZero()) throw std::domain_error("除数不能为零");
        if (isZero()) return "0.0e+0";

        // 用完整整数确定指数和两位有效数字，避免截取前缀造成舍入错误。
        int exponent = static_cast<int>(digits_.size()) -
                       static_cast<int>(divisor.digits_.size());
        std::string numerator = digits_;
        std::string denominator = divisor.digits_;
        if (exponent >= 0) denominator.append(static_cast<size_t>(exponent), '0');
        else numerator.append(static_cast<size_t>(-exponent), '0');
        UnsignedBigInteger scaledNumerator(numerator);
        const UnsignedBigInteger scaledDenominator(denominator);
        if (scaledNumerator.compare(scaledDenominator) < 0) {
            numerator.push_back('0');
            --exponent;
        }
        // 放大十倍取两位整数，按完整余数执行四舍五入。
        scaledNumerator = UnsignedBigInteger(numerator + "0");
        const auto quotient = divideStrings(scaledNumerator, scaledDenominator);
        int significant = std::stoi(quotient.first);  // 范围 10..99
        if ((UnsignedBigInteger(quotient.second) * UnsignedBigInteger("2"))
                .compare(scaledDenominator) >= 0) ++significant;
        if (significant == 100) {
            significant = 10;
            ++exponent;
        }
        return std::to_string(significant / 10) + "." +
               std::to_string(significant % 10) + "e" +
               (exponent >= 0 ? "+" : "") + std::to_string(exponent);
    }

    friend std::ostream& operator<<(std::ostream& output,
                                    const UnsignedBigInteger& number) {
        return output << number.digits_;
    }

private:
    std::string digits_;

    static std::pair<std::string, std::string> divideStrings(
        const UnsignedBigInteger& dividend, const UnsignedBigInteger& divisor) {
        if (divisor.isZero()) throw std::domain_error("除数不能为零");
        if (dividend.compare(divisor) < 0) return {"0", dividend.digits_};

        std::string quotient;
        UnsignedBigInteger remainder("0");
        for (char digit : dividend.digits_) {
            remainder = UnsignedBigInteger(remainder.digits_ + digit);
            int quotientDigit = 0;
            for (int candidate = 9; candidate >= 1; --candidate) {
                const UnsignedBigInteger product =
                    divisor * UnsignedBigInteger(std::to_string(candidate));
                if (product.compare(remainder) <= 0) {
                    remainder = remainder.subtract(product);
                    quotientDigit = candidate;
                    break;
                }
            }
            quotient.push_back(static_cast<char>('0' + quotientDigit));
        }
        const size_t first = quotient.find_first_not_of('0');
        if (first == std::string::npos) quotient = "0";
        else quotient.erase(0, first);
        return {quotient, remainder.digits_};
    }

    void normalize() {
        const size_t first = digits_.find_first_not_of('0');
        digits_ = first == std::string::npos ? "0" : digits_.substr(first);
    }


};

namespace {

std::string trim(const std::string& text) {
    const size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

bool validDigits(const std::string& text) {
    return !text.empty() &&
           std::all_of(text.begin(), text.end(),
                       [](unsigned char c) { return std::isdigit(c) != 0; });
}

bool parseExpression(const std::string& line, std::string& left, char& op,
                     std::string& right) {
    const std::string expression = trim(line);
    const size_t operatorPosition = expression.find_first_of("+-*/%");
    if (operatorPosition == std::string::npos) return false;
    left = trim(expression.substr(0, operatorPosition));
    right = trim(expression.substr(operatorPosition + 1));
    op = expression[operatorPosition];
    return validDigits(left) && validDigits(right);
}

void calculate(const std::string& leftText, char op, const std::string& rightText) {
    try {
        const UnsignedBigInteger left(leftText);
        const UnsignedBigInteger right(rightText);
        std::cout << "结果：";
        switch (op) {
            case '+':
                std::cout << left + right;
                break;
            case '-':
                if (left.compare(right) < 0) std::cout << '-';
                std::cout << (left.compare(right) >= 0 ? left - right : right - left);
                break;
            case '*':
                std::cout << left * right;
                break;
            case '/':
                std::cout << left.scientificQuotient(right);
                break;
            case '%':
                std::cout << left % right;
                break;
            default:
                std::cout << "不支持的运算符";
        }
        std::cout << '\n';
    } catch (const std::exception& error) {
        std::cout << "错误：" << error.what() << '\n';
    }
}

}  // namespace

int main() {
    std::cout << "========== 无符号大整数计算器 ==========\n"
              << "输入格式：数字 运算符 数字，例如 12345678901234567890 * 2\n"
              << "支持 +、-、*、/、%；除法结果使用两位有效数字科学计数法\n"
              << "输入 q 退出。\n";

    std::string line;
    while (true) {
        std::cout << "\n请输入表达式：";
        if (!std::getline(std::cin, line) || trim(line) == "q" || trim(line) == "Q") {
            break;
        }
        std::string left;
        std::string right;
        char op = 0;
        if (!parseExpression(line, left, op, right)) {
            std::cout << "错误：请输入两个无符号整数和一个 +、-、*、/、% 运算符。\n";
            continue;
        }
        calculate(left, op, right);
    }
    std::cout << "程序已退出。\n";
    return 0;
}
