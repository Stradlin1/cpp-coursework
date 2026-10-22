# 21 点纸牌游戏

支持庄家对战 1-7 名玩家，A 按 1 或 11 计算，J、Q、K 按 10 计算。玩家依次选择要牌或停牌，庄家点数小于等于 16 时必须要牌，最后按题目规则结算，特别是双方爆牌按课程原题判平局。

程序使用 `Participant` 抽象参与者，派生出 `HumanPlayer` 和 `Dealer`，并通过
`GameState` 管理发牌、玩家回合、庄家回合和结算阶段。牌局状态和角色行为分离，
便于后续加入自动玩家或图形化界面。

```bash
g++ -std=c++17 -Wall -Wextra -pedantic blackjack.cpp -o blackjack
./blackjack
```

## 回归测试

```bash
g++ -std=c++17 -Wall -Wextra -pedantic tests/test_round_rules.cpp -o /tmp/test_round_rules
/tmp/test_round_rules
```
