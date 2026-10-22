#!/usr/bin/env python3
"""End-to-end regression checks for employee promotion and legacy storage."""
from pathlib import Path
import subprocess
import tempfile

SOURCE = Path(__file__).resolve().parents[1] / "employee_system.cpp"


def run(exe: Path, script: str) -> str:
    result = subprocess.run([str(exe)], input=script, text=True,
                            capture_output=True, timeout=10, check=True)
    return result.stdout


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="employee_promotion_") as directory:
        root = Path(directory)
        exe = root / "employee_system"
        subprocess.run(["g++", "-std=c++17", "-Wall", "-Wextra", "-pedantic",
                        str(SOURCE), "-o", str(exe)], check=True, timeout=60)

        result = run(exe, "admin\nadmin\n4\n1\nAlice\n8\n1\n0\n")
        assert "晋升成功：Alice，级别 1 -> 4" in result
        saved = (root / "employees.dat").read_text(encoding="utf-8")
        assert '1 "Alice" 0 4' in saved, saved
        audit = (root / "employee_operations.log").read_text(encoding="utf-8")
        assert "员工晋升" in audit

        result = run(exe, "admin\nadmin\n4\n5\nBob\n1000\n8\n2\n0\n")
        assert "已达到该职位最高级别" in result
        saved = (root / "employees.dat").read_text(encoding="utf-8")
        assert '1 "Alice" 0 4' in saved, saved
        assert '5 "Bob" 1000 1' in saved, saved

        (root / "employees.dat").write_text(
            '1 "Legacy Manager" 0\n3 "Legacy Technician" 200\n', encoding="utf-8")
        run(exe, "admin\nadmin\n0\n")
        saved = (root / "employees.dat").read_text(encoding="utf-8")
        assert '1 "Legacy Manager" 0 4' in saved, saved
        assert '3 "Legacy Technician" 200 2' in saved, saved


if __name__ == "__main__":
    main()
    print("Employee promotion regression tests passed.")
