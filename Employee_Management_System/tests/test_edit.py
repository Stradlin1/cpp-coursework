#!/usr/bin/env python3
from pathlib import Path
import subprocess
import tempfile

source=Path(__file__).resolve().parents[1]/"employee_system.cpp"
with tempfile.TemporaryDirectory() as d:
    root=Path(d)
    exe=root/"employee_system"
    subprocess.run(["g++","-std=c++17","-Wall","-Wextra","-pedantic",str(source),"-o",str(exe)],check=True,timeout=60)
    # A general manager's extra field is not editable; renaming should work.
    script="admin\nadmin\n4\n1\nAlice\n5\n1\n2\n5\n1\n1\nAlicia\n4\n3\nBob\n100\n5\n2\n2\n200\n0\n"
    result=subprocess.run([str(exe)],input=script,text=True,capture_output=True,timeout=10,check=True)
    assert "该职位没有可修改的奖金或销售额" in result.stdout,result.stdout
    assert result.stdout.count("修改成功") == 2,result.stdout
    saved=(root/"employees.dat").read_text(encoding="utf-8")
    assert '1 "Alicia" 0 1' in saved,saved
    assert '3 "Bob" 200 1' in saved,saved
    assert '1 "Alice"' not in saved,saved
    print("Employee edit regression tests passed.")
