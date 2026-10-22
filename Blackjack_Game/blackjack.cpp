#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <iostream>
#include <memory>
#include <random>
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

enum class Suit { Hearts, Diamonds, Clubs, Spades };
enum class Rank { Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
                  Jack, Queen, King, Ace };

struct Card {
    Suit suit;
    Rank rank;

    int value() const {
        if (rank == Rank::Ace) return 11;
        return std::min(static_cast<int>(rank), 10);
    }

    std::string text() const {
        static const char* suits[] = {"H", "D", "C", "S"};
        static const char* ranks[] = {"", "", "2", "3", "4", "5", "6", "7",
                                      "8", "9", "10", "J", "Q", "K", "A"};
        return std::string(suits[static_cast<int>(suit)]) +
               ranks[static_cast<int>(rank)];
    }
};

class Deck {
public:
    Deck() : engine_(std::random_device{}()) { shuffle(); }

    void shuffle() {
        cards_.clear();
        for (int suit = 0; suit < 4; ++suit) {
            for (int rank = 2; rank <= 14; ++rank) {
                cards_.push_back(
                    {static_cast<Suit>(suit), static_cast<Rank>(rank)});
            }
        }
        std::shuffle(cards_.begin(), cards_.end(), engine_);
    }

    Card deal() {
        if (cards_.empty()) shuffle();
        Card card = cards_.back();
        cards_.pop_back();
        return card;
    }

private:
    std::vector<Card> cards_;
    std::mt19937 engine_;
};

struct Hand {
    std::vector<Card> cards;
    bool busted = false;

    void clear() {
        cards.clear();
        busted = false;
    }

    void add(Card card) { cards.push_back(card); }

    int score() const {
        int total = 0;
        int aces = 0;
        for (const Card& card : cards) {
            total += card.value();
            if (card.rank == Rank::Ace) ++aces;
        }
        while (total > 21 && aces > 0) {
            total -= 10;
            --aces;
        }
        return total;
    }

    void show(const std::string& owner, bool hideFirst = false) const {
        std::cout << owner << "：";
        for (size_t i = 0; i < cards.size(); ++i) {
            if (hideFirst && i == 0) std::cout << "[隐藏] ";
            else std::cout << cards[i].text() << ' ';
        }
        if (hideFirst) std::cout << "| 点数未知";
        else std::cout << "| 点数 " << score();
        if (busted) std::cout << "（爆牌）";
        std::cout << '\n';
    }
};

class Participant {
public:
    explicit Participant(std::string name) : name_(std::move(name)) {}
    virtual ~Participant() = default;

    void clearHand() { hand_.clear(); }
    void receive(Card card) { hand_.add(card); }
    const Hand& hand() const { return hand_; }
    Hand& hand() { return hand_; }
    const std::string& name() const { return name_; }
    virtual bool wantsCard() = 0;

protected:
    std::string name_;
    Hand hand_;
};

class HumanPlayer final : public Participant {
public:
    using Participant::Participant;

    bool wantsCard() override {
        while (true) {
            std::cout << "1. 要牌  2. 停牌：";
            int choice = 0;
            if (readNumber(choice) && (choice == 1 || choice == 2)) return choice == 1;
            if (std::cin.eof()) throw std::runtime_error("输入结束，牌局已取消");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入 1 或 2。\n";
        }
    }
};

class Dealer final : public Participant {
public:
    using Participant::Participant;

    bool wantsCard() override { return hand_.score() <= 16; }
};

class BlackjackGame {
public:
    void run() {
        int playerCount = readInteger("请输入玩家人数（1-7）：", 1, 7);
        for (int i = 0; i < playerCount; ++i) {
            players_.push_back(std::make_unique<HumanPlayer>("玩家" + std::to_string(i + 1)));
        }

        char again = 'y';
        while (again == 'y' || again == 'Y') {
            playRound();
            std::cout << "是否继续下一局？（y/n）：";
            if (!(std::cin >> again)) break;
        }
        std::cout << "感谢游玩。\n";
    }

private:
    enum class GameState { Setup, Dealing, PlayerTurns, DealerTurn, Settlement, Finished };

    Deck deck_;
    Dealer dealer_{"庄家"};
    std::vector<std::unique_ptr<HumanPlayer>> players_;
    GameState state_ = GameState::Setup;

    static int readInteger(const std::string& prompt, int min, int max) {
        while (true) {
            std::cout << prompt;
            int value = 0;
            if (readNumber(value) && value >= min && value <= max) return value;
            if (std::cin.eof()) throw std::runtime_error("输入结束，牌局已取消");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请输入 " << min << "-" << max << "。\n";
        }
    }

    void transitionTo(GameState next) {
        const bool valid =
            (state_ == GameState::Setup && next == GameState::Dealing) ||
            (state_ == GameState::Finished && next == GameState::Dealing) ||
            (state_ == GameState::Dealing && next == GameState::PlayerTurns) ||
            (state_ == GameState::PlayerTurns && next == GameState::DealerTurn) ||
            (state_ == GameState::DealerTurn && next == GameState::Settlement) ||
            (state_ == GameState::Settlement && next == GameState::Finished);
        if (valid) state_ = next;
    }

    void dealInitialCards() {
        transitionTo(GameState::Dealing);
        dealer_.clearHand();
        for (const auto& player : players_) player->clearHand();
        deck_.shuffle();
        for (int round = 0; round < 2; ++round) {
            for (const auto& player : players_) player->receive(deck_.deal());
            dealer_.receive(deck_.deal());
        }
    }

    void playRound() {
        dealInitialCards();
        std::cout << "\n========== 发牌 ==========\n";
        for (const auto& player : players_) player->hand().show(player->name());
        dealer_.hand().show(dealer_.name(), true);

        transitionTo(GameState::PlayerTurns);
        for (const auto& player : players_) {
            std::cout << "\n========== " << player->name() << " 回合 ==========\n";
            while (player->hand().score() < 21 && player->wantsCard()) {
                player->hand().show(player->name());
                player->receive(deck_.deal());
                if (player->hand().score() > 21) {
                    player->hand().busted = true;
                    player->hand().show(player->name());
                    break;
                }
            }
            if (player->hand().score() == 21) std::cout << "达到21点，自动停牌。\n";
        }

        transitionTo(GameState::DealerTurn);
        std::cout << "\n========== 庄家回合 ==========\n";
        dealer_.hand().show(dealer_.name());
        while (dealer_.wantsCard()) {
            dealer_.receive(deck_.deal());
            dealer_.hand().show(dealer_.name());
        }
        if (dealer_.hand().score() > 21) dealer_.hand().busted = true;
        transitionTo(GameState::Settlement);
        settle();
        transitionTo(GameState::Finished);
    }

    void settle() const {
        std::cout << "\n========== 结算 ==========\n";
        const std::string dealerResult =
            dealer_.hand().busted ? "爆牌" : std::to_string(dealer_.hand().score());
        std::cout << "庄家：" << dealerResult << '\n';
        for (const auto& player : players_) {
            std::cout << player->name() << "：";
            if (player->hand().busted && dealer_.hand().busted)
                std::cout << "平局（双方爆牌，按课程规则）";
            else if (player->hand().busted) std::cout << "失败（玩家爆牌）";
            else if (dealer_.hand().busted) std::cout << "胜利（庄家爆牌）";
            else if (player->hand().score() > dealer_.hand().score()) std::cout << "胜利";
            else if (player->hand().score() == dealer_.hand().score()) std::cout << "平局";
            else std::cout << "失败";
            std::cout << '\n';
        }
    }
};

int main() {
    BlackjackGame game;
    try { game.run(); }
    catch (const std::runtime_error& error) { std::cout << error.what() << '\n'; }
    return 0;
}
