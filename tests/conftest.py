"""Shared fixtures and helpers for the integration test suite."""

from __future__ import annotations

import os
import subprocess
from pathlib import Path
from typing import Iterable

import pytest

ROOT = Path(__file__).resolve().parents[1]


@pytest.fixture(scope="session", autouse=True)
def build_project() -> None:
    """Build every session once before the first test runs."""
    subprocess.run(["make", "-s"], cwd=ROOT, check=True)


def binary(tp: str, name: str) -> Path:
    return ROOT / tp / "bin" / name


def run(
    arguments: Iterable[str | os.PathLike[str]],
    *,
    cwd: Path | None = None,
    input_text: str | None = None,
    timeout: float = 20.0,
    env: dict[str, str] | None = None,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    """Run a program with a controlled environment and a hard timeout."""
    environment = os.environ.copy()
    environment.setdefault("PC_SLEEP_SCALE", "0.05")
    environment.setdefault("LC_ALL", "C")
    if env:
        environment.update(env)
    return subprocess.run(
        [str(argument) for argument in arguments],
        cwd=str(cwd or ROOT),
        input=input_text,
        text=True,
        capture_output=True,
        timeout=timeout,
        env=environment,
        check=check,
    )


def wait_for_output(
    process: subprocess.Popen[str],
    expected: str,
    *,
    timeout: float = 10.0,
) -> str:
    """Read stdout of a live process until expected appears or timeout."""
    import select
    import time

    assert process.stdout is not None
    collected: list[str] = []
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        ready, _, _ = select.select([process.stdout], [], [], 0.1)
        if not ready:
            continue
        line = process.stdout.readline()
        if not line:
            break
        collected.append(line)
        if expected in line:
            return "".join(collected)
    raise AssertionError(
        f"did not see {expected!r} within {timeout}s; output was {''.join(collected)!r}"
    )
