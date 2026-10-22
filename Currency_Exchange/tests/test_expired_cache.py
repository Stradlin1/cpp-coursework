#!/usr/bin/env python3
from pathlib import Path
import os
import subprocess
import tempfile
import time

source = Path(__file__).resolve().parents[1] / "currency_exchange.cpp"
with tempfile.TemporaryDirectory() as d:
    root = Path(d)
    exe = root / "currency_exchange"
    subprocess.run(["g++","-std=c++17","-Wall","-Wextra","-pedantic",str(source),"-o",str(exe)],check=True,timeout=60)
    env = dict(os.environ, CURRENCY_EXCHANGE_OFFLINE="1")
    for answer, expected in [("y","过期本地缓存"),("n","用户手动输入")]:
        (root/"exchange_rates.cache").write_text(f"USD 6.5 {int(time.time())-90000}\n",encoding="utf-8")
        script=f"1\n3\n{answer}\n"+("2\n" if answer=="y" else "6.8\n2\n")+"7\n3\n"
        result=subprocess.run([str(exe)],input=script,text=True,capture_output=True,cwd=root,env=env,timeout=10,check=True)
        assert "是否仍使用该过期汇率" in result.stdout, result.stdout
        assert expected in result.stdout, result.stdout
        assert ("13.00" if answer=="y" else "13.60") in result.stdout,result.stdout
    print("Expired cache confirmation tests passed.")
