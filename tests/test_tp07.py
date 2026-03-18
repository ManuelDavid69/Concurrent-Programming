"""TP 7 — ring of processes and pipe-based calculator."""

from __future__ import annotations

import re

import pytest

from conftest import binary, run


def parse_greater(stdout: str) -> tuple[list[tuple[int, int, int]], tuple[int, int, int]]:
    process_pattern = re.compile(r"^processus pid (\d+) numéro (\d+) val = (\d+)$")
    final_pattern = re.compile(
        r"^Le plus grand nombre = (\d+) - pid = (\d+) - Numéro (\d+)$"
    )
    processes: list[tuple[int, int, int]] = []
    final: tuple[int, int, int] | None = None
    for line in stdout.splitlines():
        match = process_pattern.match(line)
        if match:
            pid, rank, value = map(int, match.groups())
            processes.append((pid, rank, value))
            continue
        match = final_pattern.match(line)
        if match:
            value, pid, rank = map(int, match.groups())
            final = (pid, rank, value)
    assert final is not None
    return processes, final


@pytest.mark.parametrize("count", [1, 2, 3, 6, 12])
def test_ring_finds_the_maximum(count: int) -> None:
    result = run([binary("tp07", "greater"), str(count)])
    processes, final = parse_greater(result.stdout)
    assert len(processes) == count
    assert sorted(rank for _, rank, _ in processes) == list(range(count))
    values = {(pid, rank): value for pid, rank, value in processes}
    maximum = max(values.values())
    assert final[2] == maximum
    assert values[(final[0], final[1])] == maximum


def test_calculator_answers_every_client() -> None:
    result = run([binary("tp07", "calculator"), "5"], env={"CALC_SEED": "42"})
    lines = result.stdout.splitlines()
    assert len(lines) == 5
    pattern = re.compile(r"^Client (\d+): (-?\d+) ([+\-*/]) (-?\d+) = (-?\d+)$")
    seen = set()
    for line in lines:
        match = pattern.match(line)
        assert match, line
        client, left, operator, right, answer = match.groups()
        left = int(left)
        right = int(right)
        answer = int(answer)
        if operator == "+":
            assert answer == left + right
        elif operator == "-":
            assert answer == left - right
        elif operator == "*":
            assert answer == left * right
        else:
            assert right != 0
            assert answer == left // right
        seen.add(int(client))
    assert seen == set(range(5))


def test_calculator_is_reproducible_with_a_seed() -> None:
    first = run([binary("tp07", "calculator"), "3"], env={"CALC_SEED": "7"})
    second = run([binary("tp07", "calculator"), "3"], env={"CALC_SEED": "7"})
    assert sorted(first.stdout.splitlines()) == sorted(second.stdout.splitlines())


def test_calculator_rejects_an_invalid_client_count() -> None:
    result = run([binary("tp07", "calculator"), "0"], check=False)
    assert result.returncode == 1
