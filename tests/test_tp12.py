"""TP 12 — POSIX threads."""

from __future__ import annotations

import re
from pathlib import Path

import pytest

from conftest import binary, run


def test_rendezvous_with_condition_variable() -> None:
    result = run([binary("tp12", "rendezvous_sem"), "5"])
    lines = [line for line in result.stdout.splitlines() if "rendez-vous" in line]
    before = [index for index, line in enumerate(lines) if "avant" in line]
    after = [index for index, line in enumerate(lines) if "atteint" in line]
    assert len(before) == 5
    assert len(after) == 5
    assert max(before) < min(after)


def test_rendezvous_with_barrier() -> None:
    result = run([binary("tp12", "rendezvous_barrier"), "5"])
    lines = [line for line in result.stdout.splitlines() if "barrière" in line]
    before = [index for index, line in enumerate(lines) if "avant" in line]
    after = [index for index, line in enumerate(lines) if "franchie" in line]
    assert len(before) == 5
    assert len(after) == 5
    assert max(before) < min(after)


def test_producer_consumer_threads() -> None:
    result = run([binary("tp12", "prodcons"), "4", "3", "2", "25"])
    produced = [int(value) for value in re.findall(r"dépose (\d+)", result.stdout)]
    consumed = [int(value) for value in re.findall(r"récupère (\d+)", result.stdout)]
    assert len(produced) == 100
    assert sorted(produced) == sorted(consumed)
    assert "Total consommé: 100 objets (attendu 100)" in result.stdout
    assert result.returncode == 0


def test_thread_value_exchange() -> None:
    result = run([binary("tp12", "echange")])
    first = [int(value) for value in re.search(r"T1:(.*)", result.stdout).group(1).split()]
    second = [int(value) for value in re.search(r"T2:(.*)", result.stdout).group(1).split()]
    assert max(first) <= min(second)
    assert sorted(first + second) == sorted(list(range(1, 9)) + list(range(20, 91, 10)))
    assert "8 échange(s)" in result.stdout


def test_mandelbrot_is_thread_count_independent(tmp_path: Path) -> None:
    single = tmp_path / "single.ppm"
    multi = tmp_path / "multi.ppm"
    first = run([binary("tp12", "fractale"), "1", "160", "120", single])
    second = run([binary("tp12", "fractale"), "4", "160", "120", multi], timeout=60)
    assert "160x120" in first.stdout
    assert "4 thread(s)" in second.stdout
    assert single.read_bytes() == multi.read_bytes()
    data = single.read_bytes()
    assert data.startswith(b"P6")
    assert len(set(data[data.find(b"255\n") + 4 :])) > 10


def test_mandelbrot_rejects_invalid_size() -> None:
    result = run([binary("tp12", "fractale"), "2", "0", "10"], check=False)
    assert result.returncode == 1
