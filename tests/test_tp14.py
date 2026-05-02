"""TP 14 — ball manager and Monte-Carlo pi."""

from __future__ import annotations

import math
import re

from conftest import binary, run


def test_ball_manager_respects_the_invariant() -> None:
    result = run([binary("tp14", "billes"), "4", "9", "3", "4,3,5,2"], timeout=30)
    assert result.returncode == 0
    assert result.stdout.count("bille(s) obtenue(s)") == 4 * 3
    assert result.stdout.count("Travailleur") >= 8
    assert "invariant respecté" in result.stdout
    assert "invariant violé" not in result.stdout


def test_ball_manager_rejects_an_impossible_demand() -> None:
    result = run([binary("tp14", "billes"), "2", "3", "1", "5,1"], check=False)
    assert result.returncode == 1


def pi_value(output: str, label: str = "Pi") -> float:
    match = re.search(rf"{label} = ([0-9.]+)", output)
    assert match
    return float(match.group(1))


def test_monte_carlo_pi_estimate() -> None:
    result = run([binary("tp14", "pi"), "300000", "4"])
    estimate = pi_value(result.stdout)
    assert abs(estimate - math.pi) < 0.05
    assert "4 thread(s)" in result.stdout


def test_monte_carlo_compare_mode() -> None:
    result = run([binary("tp14", "pi"), "--compare", "300000", "4"])
    assert "Mono-thread: Pi = " in result.stdout
    assert "Multi-thread: Pi = " in result.stdout
    single = pi_value(result.stdout, "Mono-thread: Pi")
    multi = pi_value(result.stdout, "Multi-thread: Pi")
    assert abs(single - math.pi) < 0.05
    assert abs(multi - math.pi) < 0.05
    assert "Accélération:" in result.stdout


def test_monte_carlo_rejects_invalid_sample_count() -> None:
    result = run([binary("tp14", "pi"), "0"], check=False)
    assert result.returncode == 1
