"""TP 4 — arguments, fork, wait and process trees."""

from __future__ import annotations

import re

import pytest

from conftest import binary, run


def test_mirror_reverses_every_argument() -> None:
    result = run([binary("tp04", "miroir"), "trace", "soda", "saper"])
    assert result.stdout == "ecart ados repas\n"


def test_mirror_with_one_argument() -> None:
    assert run([binary("tp04", "miroir"), "abc"]).stdout == "cba\n"


@pytest.mark.parametrize(
    ("arguments", "expected"),
    [
        (["12", "18", "3"], "Moyenne: 11.00\n"),
        (["20", "0"], "Moyenne: 10.00\n"),
        (["15"], "Moyenne: 15.00\n"),
    ],
)
def test_average(arguments: list[str], expected: str) -> None:
    result = run([binary("tp04", "moyenne"), *arguments])
    assert result.stdout == expected


@pytest.mark.parametrize("arguments", [["12", "18", "toto"], ["12", "3.5"], ["12", "22"], ["-1"]])
def test_average_rejects_invalid_grades(arguments: list[str]) -> None:
    result = run([binary("tp04", "moyenne"), *arguments], check=False)
    assert result.returncode == 1
    assert result.stdout == "Note(s) non valide(s)\n"


def test_average_without_arguments() -> None:
    result = run([binary("tp04", "moyenne")], check=False)
    assert result.returncode == 1
    assert result.stdout == "Aucune note fourni, moyenne impossible à calculer\n"


def test_fork_loop_creates_the_complete_tree() -> None:
    result = run([binary("tp04", "boucle")])
    lines = result.stdout.splitlines()
    # The root prints at 0, 1, 2; its child at 0, 1, 2; the two
    # grandchildren at 1, 2; the four great-grandchildren at 2.
    assert len(lines) == 14
    pattern = re.compile(
        r"^([0-2]): Je suis le processus (\d+), mon père est le processus (\d+), "
        r"fork a retourné (-?\d+)$"
    )
    pids = set()
    for line in lines:
        match = pattern.match(line)
        assert match, line
        pids.add(int(match.group(2)))
    assert len(pids) == 8


def test_wait_creates_children_and_reports_exit_codes() -> None:
    result = run([binary("tp04", "wait"), "4"])
    lines = result.stdout.splitlines()
    starts = [line for line in lines if line.startswith("Fils ")]
    assert sum("je dors" in line for line in starts) == 4
    assert sum("je reprends" in line for line in starts) == 4
    parent_lines = [line for line in lines if line.startswith("Père:")]
    assert any("les 4 fils sont créés" in line for line in parent_lines)
    codes = {
        int(match.group(1))
        for line in parent_lines
        if (match := re.search(r"code (\d+)$", line))
    }
    assert codes == {0, 1, 2, 3}


def test_wait_rejects_invalid_count() -> None:
    result = run([binary("tp04", "wait"), "0"], check=False)
    assert result.returncode == 1


def test_four_children_cover_one_to_two_hundred() -> None:
    result = run([binary("tp04", "enfants")])
    values = [int(line) for line in result.stdout.splitlines()]
    assert len(values) == 200
    assert sorted(values) == list(range(1, 201))


def test_ordered_children_print_exactly_one_to_two_hundred() -> None:
    result = run([binary("tp04", "enfants"), "--ordered"])
    values = [int(line) for line in result.stdout.splitlines()]
    assert values == list(range(1, 201))


def test_analysis_case_one_prints_the_expected_set() -> None:
    result = run([binary("tp04", "analyse-cas1")])
    values = [int(line) for line in result.stdout.splitlines()]
    assert sorted(values) == [0, 2, 4, 6, 8]


def test_analysis_case_two_prints_five_letters() -> None:
    result = run([binary("tp04", "analyse-cas2")])
    letters = {line.split()[2].rstrip(",") for line in result.stdout.splitlines()}
    assert letters == {"A", "B", "C", "D", "E"}


def test_analysis_case_two_ordered_is_reverse_alphabetical() -> None:
    result = run([binary("tp04", "analyse-cas2-ordonne")])
    assert [line.split()[2] for line in result.stdout.splitlines()] == ["E", "D", "C", "B", "A"]


def test_analysis_case_three_sums_and_parent_message() -> None:
    result = run([binary("tp04", "analyse-cas3")])
    lines = result.stdout.splitlines()
    assert lines[-1] == "Calculs terminés"
    sums = {}
    for line in lines[:-1]:
        index, value = map(int, line.split())
        sums[index] = value
    assert sums == {0: 100, 1: 125, 2: 100, 3: 125}
