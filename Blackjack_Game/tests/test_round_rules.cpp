#include "../round_rules.h"
#include <cassert>

int main() {
    assert(judgeRound(23, 25) == RoundOutcome::Draw);
    assert(judgeRound(22, 22) == RoundOutcome::Draw);
    assert(judgeRound(20, 25) == RoundOutcome::Win);
    assert(judgeRound(23, 20) == RoundOutcome::Lose);
    assert(judgeRound(21, 20) == RoundOutcome::Win);
    assert(judgeRound(18, 20) == RoundOutcome::Lose);
    assert(judgeRound(20, 20) == RoundOutcome::Draw);
    return 0;
}
