#!/usr/bin/env python3
from pathlib import Path
import subprocess
import tempfile
source=Path(__file__).resolve().parents[1]/"blackjack.cpp"
with tempfile.TemporaryDirectory() as d:
    exe=str(Path(d)/"blackjack")
    subprocess.run(["g++","-std=c++17","-Wall","-Wextra","-pedantic",str(source),"-o",exe],check=True,timeout=60)
    # One player, keep requesting cards until busted or 21; all new hands must be
    # displayed before the next decision prompt.
    result=subprocess.run([exe],input="1\n"+"1\n"*30+"n\n",text=True,capture_output=True,timeout=10,check=True)
    out=result.stdout
    assert "========== 玩家1 回合 ==========" in out,out
    turn=out.split("========== 玩家1 回合 ==========")[1].split("========== 庄家回合 ==========")[0]
    if turn.count("1. 要牌  2. 停牌：")>1:
        first=turn.index("1. 要牌  2. 停牌：")
        second=turn.index("1. 要牌  2. 停牌：",first+1)
        assert "| 点数 " in turn[first:second],turn
    print("Blackjack hit-display regression test passed.")
