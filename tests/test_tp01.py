"""TP 1 — C refresher."""

from __future__ import annotations

import pytest

from conftest import binary, run


def test_prime_factorisation_of_the_statement_example() -> None:
    result = run([binary("tp01", "facteurs"), "1170701532"])
    assert result.stdout.strip() == (
        "1170701532 = 2 x 2 x 3 x 3 x 3 x 7 x 7 x 7 x 11 x 13 x 13 x 17"
    )


@pytest.mark.parametrize(
    ("number", "expected"),
    [
        ("2", "2 = 2"),
        ("97", "97 = 97"),
        ("360", "360 = 2 x 2 x 2 x 3 x 3 x 5"),
        ("1", "1 = 1"),
        ("0", "0 = 0"),
        ("-12", "-12 = -1 x 2 x 2 x 3"),
    ],
)
def test_prime_factorisation_edge_cases(number: str, expected: str) -> None:
    result = run([binary("tp01", "facteurs"), number])
    assert result.stdout.strip() == expected


def test_prime_factorisation_handles_several_arguments() -> None:
    result = run([binary("tp01", "facteurs"), "12", "13"])
    assert result.stdout.splitlines() == ["12 = 2 x 2 x 3", "13 = 13"]


def test_look_and_say_matches_the_statement() -> None:
    result = run([binary("tp01", "enigme"), "3", "7"])
    assert result.stdout.splitlines() == [
        "3",
        "1 3",
        "1 1 1 3",
        "3 1 1 3",
        "1 3 2 1 1 3",
        "1 1 1 3 1 2 2 1 1 3",
        "3 1 1 3 1 1 2 2 2 1 1 3",
    ]


def test_look_and_say_from_an_arbitrary_line() -> None:
    result = run([binary("tp01", "enigme"), "1 2 1 1", "3"])
    assert result.stdout.splitlines() == ["1 2 1 1", "1 1 1 2 2 1", "3 1 2 2 1 1"]


def test_line_intersection_inside_both_segments() -> None:
    result = run(
        [binary("tp01", "intersection"), "0", "0", "4", "4", "0", "4", "4", "0"]
    )
    assert "Intersection des droites: (2.000000, 2.000000)" in result.stdout
    assert "appartient aux deux segments" in result.stdout


def test_line_intersection_outside_segments() -> None:
    result = run(
        [binary("tp01", "intersection"), "0", "0", "1", "1", "0", "2", "1", "4"]
    )
    assert "Intersection des droites: (-2.000000, -2.000000)" in result.stdout
    assert "hors de l'un des deux segments" in result.stdout


def test_parallel_lines() -> None:
    result = run(
        [binary("tp01", "intersection"), "0", "0", "1", "1", "0", "1", "1", "2"]
    )
    assert "Aucune intersection" in result.stdout


def test_confounded_lines() -> None:
    result = run(
        [binary("tp01", "intersection"), "0", "0", "1", "1", "2", "2", "3", "3"]
    )
    assert "Infinité d'intersections" in result.stdout


def test_vertical_and_horizontal_lines() -> None:
    result = run(
        [binary("tp01", "intersection"), "2", "0", "2", "5", "0", "3", "5", "3"]
    )
    assert "Intersection des droites: (2.000000, 3.000000)" in result.stdout


def test_two_vertical_lines_are_parallel() -> None:
    result = run(
        [binary("tp01", "intersection"), "2", "0", "2", "5", "3", "0", "3", "5"]
    )
    assert "Aucune intersection" in result.stdout


@pytest.mark.parametrize(
    ("arguments", "expected"),
    [
        (["5", "2", "4", "6", "1", "3"], "1 2 3 4 5 6"),
        (["3", "3", "1", "3", "2"], "1 2 3 3 3"),
        (["-1", "5", "0", "-3"], "-3 -1 0 5"),
        (["42"], "42"),
    ],
)
def test_insertion_sort(arguments: list[str], expected: str) -> None:
    result = run([binary("tp01", "tri_insertion"), *arguments])
    assert result.stdout.splitlines()[-1] == f"Après: {expected}"


def test_insertion_sort_verbose_prints_every_insertion() -> None:
    result = run([binary("tp01", "tri_insertion"), "--verbose", "5", "2", "4", "6", "1", "3"])
    assert result.stdout.splitlines() == [
        "Avant: 5 2 4 6 1 3",
        "Après insertion 1: 2 5 4 6 1 3",
        "Après insertion 2: 2 4 5 6 1 3",
        "Après insertion 3: 2 4 5 6 1 3",
        "Après insertion 4: 1 2 4 5 6 3",
        "Après insertion 5: 1 2 3 4 5 6",
        "Après: 1 2 3 4 5 6",
    ]
