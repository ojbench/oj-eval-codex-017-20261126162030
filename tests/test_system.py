"""Deterministic integration tests, with isolated persistent storage."""
import pathlib
import random
import subprocess
import tempfile

EXE = pathlib.Path(__file__).resolve().parents[1] / "code"


def run(directory, commands):
    result = subprocess.run([str(EXE)], input="\n".join(commands) + "\n",
                            text=True, capture_output=True, cwd=directory, check=True)
    return result.stdout.splitlines()


with tempfile.TemporaryDirectory() as directory:
    assert run(directory, [
        "add_user -u root -p secret -n Root -m root@mail -g 1",
        "login -u root -p secret",
        "add_train -i T -n 3 -m 10 -s A|B|C -p 4|6 -x 23:00 -t 120|60 -o 10 -d 06-01|06-02 -y G",
        "release_train -i T",
        "buy_ticket -u root -i T -d 06-02 -n 7 -f B -t C",
        "buy_ticket -u root -i T -d 06-01 -n 5 -f A -t C -q true",
        "buy_ticket -u root -i T -d 06-01 -n 3 -f A -t C -q true",
        "refund_ticket -u root -n 3",
        "query_order -u root", "exit",
    ]) == ["0", "0", "0", "0", "42", "queue", "30", "0", "3",
           "[success] T A 06-01 23:00 -> C 06-02 02:10 10 3",
           "[success] T A 06-01 23:00 -> C 06-02 02:10 10 5",
           "[refunded] T B 06-02 01:10 -> C 06-02 02:10 6 7", "bye"]
    assert run(directory, ["query_order -u root", "login -u root -p secret",
                           "query_ticket -s B -t C -d 06-02", "exit"]) == [
        "-1", "0", "1", "T B 06-02 01:10 -> C 06-02 02:10 6 2", "bye"]
    assert run(directory, [
        "add_train -i U -n 2 -m 20 -s C|D -p 7 -x 02:10 -t 50 -o _ -d 06-01|06-03 -y D",
        "release_train -i U", "query_transfer -s A -t D -d 06-01", "exit"
    ]) == ["0", "0", "T A 06-01 23:00 -> C 06-02 02:10 10 2",
           "U C 06-02 02:10 -> D 06-02 03:00 7 20", "bye"]

# Compare randomized purchases/refunds against a small independent seat model.
random.seed(1170)
with tempfile.TemporaryDirectory() as directory:
    commands = ["add_user -u r -p secret -n R -m r@mail", "login -u r -p secret",
                "add_train -i T -n 4 -m 20 -s A|B|C|D -p 2|3|5 -x 08:00 -t 60|60|60 -o 0|0 -d 06-01|06-01 -y G",
                "release_train -i T"]
    expected = ["0"] * 4
    seats = [20] * 3
    orders = []
    prices = [0, 2, 5, 10]
    for _ in range(1500):
        if orders and random.random() < .4:
            nth = random.randint(1, len(orders))
            commands.append(f"refund_ticket -u r -n {nth}")
            order = orders[-nth]
            if order[3] == 2:
                expected.append("-1")
                continue
            a, b, num, status = order
            order[3] = 2
            if status == 0:
                for j in range(a, b):
                    seats[j] += num
                for pending in orders:
                    a, b, num, status = pending
                    if status == 1 and min(seats[a:b]) >= num:
                        pending[3] = 0
                        for j in range(a, b):
                            seats[j] -= num
            expected.append("0")
        else:
            a = random.randrange(3)
            b = random.randrange(a + 1, 4)
            num = random.randint(1, 23)
            queue = random.choice([True, False])
            commands.append(f"buy_ticket -u r -i T -d 06-01 -n {num} -f {'ABCD'[a]} -t {'ABCD'[b]} -q {str(queue).lower()}")
            if num > 20 or (min(seats[a:b]) < num and not queue):
                expected.append("-1")
            elif min(seats[a:b]) >= num:
                orders.append([a, b, num, 0])
                for j in range(a, b):
                    seats[j] -= num
                expected.append(str(num * (prices[b] - prices[a])))
            else:
                orders.append([a, b, num, 1])
                expected.append("queue")
    assert run(directory, commands) == expected
print("Integration and 1500 randomized transaction tests passed.")
