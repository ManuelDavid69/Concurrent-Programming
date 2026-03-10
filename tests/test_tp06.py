"""TP 6 — anonymous pipes, redirections and filters."""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from conftest import binary, run


def test_guess_finds_the_secret() -> None:
    result = run(
        [binary("tp06", "guess")],
        input_text="10\n20\n30\n40\n42\n",
        env={"GUESS_SECRET": "42"},
    )
    assert result.returncode == 0
    assert result.stdout.count("Trop petit") == 4
    assert "Trouvé !" in result.stdout


def test_guess_reports_a_number_that_is_too_large() -> None:
    result = run(
        [binary("tp06", "guess")],
        input_text="50\n42\n",
        env={"GUESS_SECRET": "42"},
    )
    assert "Trop grand" in result.stdout
    assert "Trouvé !" in result.stdout


def test_guess_rejects_invalid_input() -> None:
    result = run(
        [binary("tp06", "guess")],
        input_text="abc\n9999\n42\n",
        env={"GUESS_SECRET": "42"},
    )
    assert "Entrez un entier entre 1 et 1000." in result.stdout
    assert "Trouvé !" in result.stdout


def test_simple_redirection_matches_wc() -> None:
    data = Path("/etc/passwd").read_bytes()
    result = run([binary("tp06", "redirection"), "simple"])
    lines, words, bytes_count = (int(value) for value in result.stdout.split())
    assert lines == data.count(b"\n")
    assert words == len(data.split())
    assert bytes_count == len(data)


def test_simple_pipe_matches_cut_and_tr() -> None:
    result = run([binary("tp06", "redirection"), "pipe"])
    expected = "\n".join(
        line.split(":", 1)[0].upper()
        for line in Path("/etc/passwd").read_text(encoding="utf-8", errors="replace").splitlines()
    )
    assert result.stdout.strip().splitlines() == expected.splitlines()


def test_full_pipeline_matches_the_shell(tmp_path: Path) -> None:
    result = run([binary("tp06", "redirection"), "pipeline"], cwd=tmp_path)
    assert result.returncode == 0
    produced = (tmp_path / "users").read_text(encoding="utf-8")
    expected = subprocess.run(
        "cut -f 1,3 -d : < /etc/passwd | sed 's+^\\(.*\\):\\(.*\\)+\\2:\\1+' | sort -n",
        shell=True,
        text=True,
        capture_output=True,
        check=True,
    ).stdout
    assert produced == expected


@pytest.mark.parametrize("count", ["0", "1", "20", "1000"])
def test_even_odd_filter_matches_a_direct_calculation(count: str) -> None:
    result = run([binary("tp06", "somme"), "--verbose", count, "12345"])
    values = [
        int(value)
        for value in next(
            line for line in result.stdout.splitlines() if line.startswith("Nombres générés:")
        ).split(":", 1)[1].split()
    ]
    even = sum(value for value in values if value % 2 == 0)
    odd = sum(value for value in values if value % 2 == 1)
    assert f"Somme des pairs: {even}" in result.stdout
    assert f"Somme des impairs: {odd}" in result.stdout
    assert f"Somme totale: {even + odd}" in result.stdout
