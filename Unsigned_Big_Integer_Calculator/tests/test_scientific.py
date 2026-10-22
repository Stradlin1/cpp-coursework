#!/usr/bin/env python3
from pathlib import Path
import subprocess
import tempfile

source = Path(__file__).resolve().parents[1] / "calculator.cpp"
cases = [
    ("999 / 100", "1.0e+1"),
    ("100 / 3", "3.3e+1"),
    ("1 / 2000", "5.0e-4"),
    ("1 / 3", "3.3e-1"),
    ("0 / 19", "0.0e+0"),
    ("12345 / 1000", "1.2e+1"),
    ("995 / 100", "1.0e+1"),
    ("994 / 100", "9.9e+0"),
    ("1 / 0", "错误：除数不能为零"),
    ("9" * 80 + " / 1", "1.0e+80"),
]
with tempfile.TemporaryDirectory() as d:
    exe = str(Path(d)/"calculator")
    subprocess.run(["g++","-std=c++17","-Wall","-Wextra","-pedantic",str(source),"-o",exe],check=True,timeout=60)
    inp="\n".join(expression for expression,_ in cases)+"\nq\n"
    result=subprocess.run([exe],input=inp,text=True,capture_output=True,timeout=20,check=True)
    outputs=[part.split("\n")[0].strip() for part in result.stdout.split("结果：")[1:]]
    # Divide-by-zero error is prefixed with 结果： before the catch prints 错误.
    assert len(outputs)==len(cases), (len(outputs),result.stdout)
    for (expression,expected),got in zip(cases,outputs):
        assert expected in got,(expression,expected,got)
    print("Scientific quotient regression tests passed.")
