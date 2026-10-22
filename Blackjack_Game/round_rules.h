#pragma once

// 按课程题目中的 21 点结算规则判断胜负。
// 特别注意：玩家和庄家同时爆牌时，题目要求判为平局。
enum class RoundOutcome { Win, Lose, Draw };

inline RoundOutcome judgeRound(int playerScore, int dealerScore) {
    const bool playerBusted = playerScore > 21;
    const bool dealerBusted = dealerScore > 21;
    if (playerBusted && dealerBusted) return RoundOutcome::Draw;
    if (playerBusted) return RoundOutcome::Lose;
    if (dealerBusted) return RoundOutcome::Win;
    if (playerScore > dealerScore) return RoundOutcome::Win;
    if (playerScore == dealerScore) return RoundOutcome::Draw;
    return RoundOutcome::Lose;
}
