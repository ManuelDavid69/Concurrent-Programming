"""TP 11 — shared-memory calculator and parallel image processing."""

from __future__ import annotations

import os
import re
import signal
import subprocess
import time
from collections.abc import Iterator
from pathlib import Path

import pytest

from conftest import binary, run

ENV = {
    "PC_SLEEP_SCALE": "0.05",
    "LC_ALL": "C",
    "PATH": "/usr/bin:/bin",
}


@pytest.fixture(autouse=True)
def resources() -> Iterator[None]:
    run([binary("tp11", "clean")], check=False)
    run([binary("tp11", "init")])
    try:
        yield
    finally:
        run([binary("tp11", "clean")], check=False)


def spawn(name: str, *arguments: str, env: dict[str, str] | None = None) -> subprocess.Popen[str]:
    environment = os.environ.copy()
    environment.update(ENV)
    if env:
        environment.update(env)
    return subprocess.Popen(
        [str(binary("tp11", name)), *arguments],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=environment,
    )


def test_calculator_serves_several_clients() -> None:
    servers = [spawn("serveur"), spawn("serveur")]
    clients = [
        spawn("client", "4", env={"CALC_SEED": "42"}),
        spawn("client", "4", env={"CALC_SEED": "7"}),
    ]
    try:
        time.sleep(0.1)
        outputs = [client.communicate(timeout=30)[0] for client in clients]
        for output in outputs:
            assert output.count("=") >= 4
        for server in servers:
            server.send_signal(signal.SIGINT)
        for server in servers:
            server.communicate(timeout=10)
            assert server.returncode == 0
    finally:
        for process in [*clients, *servers]:
            if process.poll() is None:
                process.kill()

    pattern = re.compile(r"Client (\d+) \((\d+)\): (-?\d+) ([+\-*/]) (-?\d+) = (-?\d+)")
    results = []
    for output in outputs:
        for line in output.splitlines():
            match = pattern.match(line)
            assert match, line
            _, _, left, operator, right, answer = match.groups()
            left_value = int(left)
            right_value = int(right)
            answer_value = int(answer)
            if operator == "+":
                assert answer_value == left_value + right_value
            elif operator == "-":
                assert answer_value == left_value - right_value
            elif operator == "*":
                assert answer_value == left_value * right_value
            else:
                assert answer_value == left_value // right_value
            results.append(answer_value)
    assert len(results) == 8


def test_calculator_rejects_an_invalid_request_count() -> None:
    result = run([binary("tp11", "client"), "0"], check=False)
    assert result.returncode == 1


def write_ppm(path: Path, width: int, height: int) -> None:
    pixels = " ".join(["255 0 0"] * (width * height))
    path.write_text(f"P3\n{width} {height}\n255\n{pixels}\n", encoding="ascii")


def read_ppm_head(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    assert data.startswith(b"P6")
    tokens = data.split(maxsplit=4)
    return int(tokens[1]), int(tokens[2])


def test_parallel_image_processing(tmp_path: Path) -> None:
    source = tmp_path / "images"
    output = tmp_path / "out"
    source.mkdir()
    images = []
    for index in range(5):
        path = source / f"image-{index}.ppm"
        write_ppm(path, 4, 4)
        images.append(path)

    servers = [spawn("serveur_image", str(output), "--drain"), spawn("serveur_image", str(output), "--drain")]
    try:
        time.sleep(0.1)
        client = spawn("client_image", str(output), *[str(path) for path in images])
        client_output = client.communicate(timeout=30)[0]
        assert client.returncode == 0, client_output
        for server in servers:
            server.communicate(timeout=30)
            assert server.returncode == 0
    finally:
        for process in servers:
            if process.poll() is None:
                process.kill()

    for index in range(5):
        destination = output / f"image-{index}.ppm"
        assert destination.exists()
        assert read_ppm_head(destination) == (4, 4)
        assert destination.stat().st_size > 20
