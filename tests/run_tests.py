#!/usr/bin/env python3
"""离线回归测试：Python 3 + g++，所有构建和运行数据放在临时目录。"""
from pathlib import Path
import decimal
import os
import random
import shutil
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
CHECKS = 0

def check(condition, message):
    global CHECKS
    assert condition, message
    CHECKS += 1

def compile_cpp(source, output):
    subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-pedantic',
                    str(source), '-o', str(output)], check=True)

def run(binary, data='', cwd=None, env=None):
    return subprocess.run([str(binary)], input=data, text=True, capture_output=True,
                          timeout=10, cwd=cwd, env=env, check=True).stdout

with tempfile.TemporaryDirectory(prefix='cpp-coursework-') as tmp:
    work = Path(tmp)
    binaries = {}
    for source in ROOT.glob('*/*.cpp'):
        binary = work / source.stem
        compile_cpp(source, binary)
        binaries[source.stem] = binary
        check(True, 'compile')

    # Python 整数及 Decimal 作独立参考，包含长数字与舍入边界。
    random.seed(20261022)
    cases = [('9999', '/', '1000'), ('999', '/', '100'), ('1999', '/', '1000'),
             ('1', '/', '2000'), ('1', '/', '9999'), ('0', '/', '1')]
    for _ in range(100):
        a = random.randrange(10**90)
        b = random.randrange(1, 10**60)
        for op in '+-*/%': cases.append((str(a), op, str(b)))
    output = run(binaries['calculator'], '\n'.join(' '.join(x) for x in cases)+'\nq\n')
    results = [line.split('结果：', 1)[1] for line in output.splitlines() if '结果：' in line]
    check(len(results) == len(cases), 'calculator result count')
    decimal.getcontext().prec = 240
    for (a, op, b), actual in zip(cases, results):
        x, y = int(a), int(b)
        if op == '/':
            if x == 0: expected = '0.0e+0'
            else:
                d = decimal.Decimal(a) / decimal.Decimal(b)
                exp = d.adjusted()
                sig = d.scaleb(1-exp).quantize(decimal.Decimal(1), rounding=decimal.ROUND_HALF_UP)
                if sig == 100: sig, exp = decimal.Decimal(10), exp+1
                expected = f'{int(sig)//10}.{int(sig)%10}e{exp:+d}'
        else:
            expected = str({'+': lambda: x+y, '-': lambda: x-y, '*': lambda: x*y,
                            '%': lambda: x%y}[op]())
        check(actual == expected, f'{a} {op} {b}: {actual} != {expected}')
    output = run(binaries['calculator'], '10/0\n-1+2\n1+2+3\n0001+0002\nq\n')
    check('除数不能为零' in output and '结果：3' in output, 'calculator errors/recovery')

    currency = binaries['currency_exchange']
    env = dict(os.environ, CURRENCY_EXCHANGE_OFFLINE='1')
    cache = work/'exchange_rates.cache'
    now = int(time.time())
    for age, usable in [(0, True), (86390, True), (86410, False), (-100, False)]:
        cache.write_text(f'broken record\nUSD 7 {now-age}\n')
        out = run(currency, '1\n3\n1\n'+('2\n' if usable else '7\n2\n')+'7\n3\n', work, env)
        check(('汇率来源：本地缓存' in out) == usable, f'cache age {age}')
        check('兑换结果：14.00' in out, 'conversion fallback')
    cache.unlink()
    out = run(currency, '1\n3\n1\n7\n2\n7\n3\n', work, env)
    check('没有找到' in out and '用户手动输入' in out, 'missing cache')
    out = run(currency, '2\n3\n2\n7\n14\n7\n3\n', work, env)
    check('兑换结果：2.00' in out, 'manual inverse conversion')
    out = run(currency, '1\n3\n2\n2\n1e308\n7\n3\n', work, env)
    check('超出可表示范围' in out, 'conversion overflow')
    out = run(currency, '1abc\n3\n', work, env)
    check('输入无效' in out, 'strict currency menu')
    for data in ['', '1\n', '1\n3\n', '1\n3\n1\n', '1\n3\n2\n7\n']:
        run(currency, data, work, env)
        check(True, 'currency EOF')

    # 用本地 curl 替身检查真实进程调用、解析和缓存写入；不冒充外网测试。
    if os.name != 'nt':
        fake = work/'fakebin'; fake.mkdir()
        curl = fake/'curl'
        curl.write_text('#!/bin/sh\nprintf \'%s\' "$MOCK_RATE_RESPONSE"\nexit "${MOCK_EXIT:-0}"\n')
        curl.chmod(0o755)
        mock_env = dict(os.environ, PATH=str(fake)+os.pathsep+os.environ['PATH'],
                        CURRENCY_EXCHANGE_OFFLINE='0')
        for response, online in [('{"CNY":7e0}', True), ('{"CNY":-1}', False),
                                 ('{"CNY":7oops}', False), ('{}', False),
                                 ('{"CNY":1e999}', False)]:
            cache.unlink(missing_ok=True)
            e = dict(mock_env, MOCK_RATE_RESPONSE=response)
            out = run(currency, '1\n3\n1\n'+('2\n' if online else '7\n2\n')+'7\n3\n', work, e)
            check(('汇率来源：在线 API' in out) == online, 'mock API parser')
            check(cache.exists() == online, 'cache only online success')
        # write/read 保留精度及小正数，不应被 to_string 舍成 0。
        e = dict(mock_env, MOCK_RATE_RESPONSE='{"CNY":0.000000123456789}')
        run(currency, '1\n3\n1\n1\n7\n3\n', work, e)
        check(float(cache.read_text().split()[1]) == 0.000000123456789, 'cache precision')

    # 确定性对象测试：不依赖随机发牌碰巧覆盖所有胜负分支。
    game_source = (ROOT/'Blackjack_Game/blackjack.cpp').read_text().replace('private:', 'public:')
    harness = work/'game_test.cpp'
    harness.write_text('#define main coursework_main\n'+game_source+'\n#undef main\n'+r'''
#include <cassert>
int main() {
    Hand hand;
    hand.add({Suit::Hearts, Rank::Ace}); hand.add({Suit::Spades, Rank::Ace});
    hand.add({Suit::Clubs, Rank::Nine}); assert(hand.score() == 21);
    Dealer dealer("D"); dealer.receive({Suit::Hearts, Rank::Ten});
    dealer.receive({Suit::Clubs, Rank::Six}); assert(dealer.wantsCard());
    dealer.receive({Suit::Clubs, Rank::Ace}); assert(!dealer.wantsCard());
    BlackjackGame game;
    auto player = std::make_unique<HumanPlayer>("P");
    player->receive({Suit::Hearts, Rank::Ten}); player->receive({Suit::Hearts, Rank::Nine});
    game.players_.push_back(std::move(player));
    game.dealer_.receive({Suit::Hearts, Rank::Ten});
    game.dealer_.receive({Suit::Hearts, Rank::Nine});
    game.settle();  // 相同点数
    game.players_[0]->receive({Suit::Hearts, Rank::Ace}); game.settle(); // 20 > 19
    game.players_[0]->hand().busted = true; game.settle(); // 玩家爆牌
    game.dealer_.hand().busted = true; game.settle(); // 双方爆牌
    game.players_[0]->hand().busted = false; game.settle(); // 庄家爆牌
}
''')
    test = work/'game_test'; compile_cpp(harness, test)
    out = run(test)
    for text in ['平局', '胜利', '失败（玩家爆牌）', '平局（双方爆牌', '胜利（庄家爆牌）']:
        check(text in out, 'blackjack '+text)
    for data in ['', '1\n', 'abc\n', '8\n']:
        run(binaries['blackjack'], data)
        check(True, 'blackjack EOF')

    employee = binaries['employee_system']
    datafile = work/'employees.dat'
    out = run(employee, 'admin\nadmin\nx\n4\n3\nAlice Smith\n', work)
    check('输入结束' in out and not datafile.exists(), 'incomplete employee must not persist')
    out = run(employee, 'admin\nadmin\n4\n3\nAlice Smith\n-1\n200\n4\n5\nBob\n100000\n4\n4\nManager\n1\n3\n0\n', work)
    for text in ['6200.00', '1000.00', '8500.00', '15700.00']:
        check(text in out, 'employee salary '+text)
    before = datafile.read_bytes()
    out = run(employee, 'user\nuser\n4\n1\n2\n2\nAlice Smith\n0\n', work)
    check('权限不足' in out and 'Alice Smith' in out, 'viewer permission/query')
    check(datafile.read_bytes() == before, 'read-only session must not rewrite data')
    # 独立行恢复，坏行不影响后续好行，读取时不静默覆写文件。
    datafile.write_text('bad\n3 "Alice Smith" 200\n5 "Bob" 100000\n')
    before = datafile.read_bytes()
    out = run(employee, 'user\nuser\n1\n0\n', work)
    check('Alice Smith' in out and 'Bob' in out, 'corrupt line recovery')
    check(datafile.read_bytes() == before, 'preserve source on read-only load')
    out = run(employee, 'a\nb\na\nb\na\nb\n', work)
    check('登录失败次数过多' in out, 'login attempt limit')
    source = work/'employee_test.cpp'
    source.write_text('#define main coursework_main\n'+
        (ROOT/'Employee_Management_System/employee_system.cpp').read_text()+
        '\n#undef main\n'+r'''
#include <cassert>
int main() {
    GeneralManager raw("A"); assert(raw.level() == 1);
    raw.promote(); assert(raw.level() == 4);
    const int levels[] = {4,3,2,3,1};
    const double wages[] = {20000,12200,6200,8500,2};
    for (int i = 1; i <= 5; ++i) {
        auto e = EmployeeFactory::create(static_cast<Role>(i), "A");
        e->setExtraValue(200); assert(e->level() == levels[i-1]);
        assert(e->salary(100000) == wages[i-1]);
    }
}
''')
    test = work/'employee_test'; compile_cpp(source, test); run(test)
    check(True, 'all role wages and initial/promotion levels')
print(f'PASS: {CHECKS} checks (offline; live API and MSVC not exercised)')
