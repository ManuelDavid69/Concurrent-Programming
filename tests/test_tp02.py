"""TP 2 — binary heap and heap sort."""

from __future__ import annotations

import pytest

from conftest import binary, run


def extraction_order(stdout: str) -> list[int]:
    line = next(line for line in stdout.splitlines() if line.startswith("Ordre d'extraction:"))
    return [int(value) for value in line.split(":", 1)[1].split()]


def test_default_demo_extracts_in_increasing_order() -> None:
    result = run([binary("tp02", "tas")])
    assert extraction_order(result.stdout) == [38, 39, 40, 42, 48, 49, 51, 55, 62]
    assert "Tas (arbre):" in result.stdout
    assert "Tas (niveaux):" in result.stdout


def test_heap_display_has_three_formats() -> None:
    result = run([binary("tp02", "tas"), "--display", "line"])
    assert "Tas (linéaire): 38 39 42 40 49 62 48 55 51" in result.stdout
    assert "Tas (arbre):" not in result.stdout

    result = run([binary("tp02", "tas"), "--display", "tree"])
    assert "Tas (arbre):" in result.stdout
    assert "Tas (linéaire):" not in result.stdout


def test_random_heap_produces_sorted_extraction() -> None:
    result = run([binary("tp02", "tas"), "--random", "40", "1234"])
    order = extraction_order(result.stdout)
    assert order == sorted(order)
    assert len(order) == 40


def test_random_generation_is_reproducible() -> None:
    first = run([binary("tp02", "tas"), "--random", "10", "99"])
    second = run([binary("tp02", "tas"), "--random", "10", "99"])
    assert first.stdout == second.stdout


@pytest.mark.parametrize(
    ("values", "expected"),
    [
        (["7", "3", "9", "1", "5"], "1 3 5 7 9"),
        (["4", "4", "4"], "4 4 4"),
        (["-5", "8", "0", "-2"], "-5 -2 0 8"),
        (["42"], "42"),
    ],
)
def test_heap_sort(values: list[str], expected: str) -> None:
    result = run([binary("tp02", "tas"), "--sort", *values])
    assert result.stdout.splitlines()[-1] == f"Après: {expected}"


def test_heap_sort_on_large_random_array() -> None:
    result = run([binary("tp02", "tas"), "--sort", "--random", "200", "7"])
    line = result.stdout.splitlines()[-1]
    values = [int(value) for value in line.split(":", 1)[1].split()]
    assert values == sorted(values)
