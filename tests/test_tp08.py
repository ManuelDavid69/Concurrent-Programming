"""TP 8 — System V semaphores."""

from __future__ import annotations

import os
import subprocess
import tempfile
import time
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

import pytest

from conftest import binary, run

TP08_ENV = {
    "PC_SLEEP_SCALE": "0.05",
    "LC_ALL": "C",
}


@dataclass
class Running:
    process: subprocess.Popen[bytes]
    stdout_path: Path
    stderr_path: Path

    def poll(self) -> int | None:
        return self.process.poll()

    def kill(self) -> None:
        if self.process.poll() is None:
            self.process.kill()
        self.process.wait()
        self.cleanup()

    def cleanup(self) -> None:
        self.stdout_path.unlink(missing_ok=True)
        self.stderr_path.unlink(missing_ok=True)


@pytest.fixture(autouse=True)
def resources() -> Iterator[None]:
    run([binary("tp08", "init")])
    try:
        yield
    finally:
        run([binary("tp08", "clean")], check=False)


def spawn(name: str, *arguments: str) -> Running:
    environment = os.environ.copy()
    environment.update(TP08_ENV)
    stdout_fd, stdout_name = tempfile.mkstemp(prefix="tp08-out-")
    stderr_fd, stderr_name = tempfile.mkstemp(prefix="tp08-err-")
    stdout_file = os.fdopen(stdout_fd, "wb")
    stderr_file = os.fdopen(stderr_fd, "wb")
    try:
        process = subprocess.Popen(
            [str(binary("tp08", name)), *arguments],
            stdout=stdout_file,
            stderr=stderr_file,
            env=environment,
        )
    finally:
        stdout_file.close()
        stderr_file.close()
    return Running(process, Path(stdout_name), Path(stderr_name))


def output_of(running: Running) -> str:
    try:
        return running.stdout_path.read_text(encoding="utf-8", errors="replace")
    except FileNotFoundError:
        return ""


def read_until(running: Running, expected: str, timeout: float = 10.0) -> str:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        output = output_of(running)
        if expected in output:
            return output
        if running.poll() is not None:
            raise AssertionError(
                f"process exited before {expected!r}; output was {output!r}, "
                f"stderr was {running.stderr_path.read_text(encoding='utf-8', errors='replace')!r}"
            )
        time.sleep(0.02)
    raise AssertionError(f"did not see {expected!r}; got {output_of(running)!r}")


def finish(running: Running, timeout: float = 15.0) -> str:
    try:
        running.process.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        running.kill()
        raise
    output = output_of(running)
    error = running.stderr_path.read_text(encoding="utf-8", errors="replace")
    assert running.process.returncode == 0, error
    running.cleanup()
    return output


def assert_blocked(running: Running) -> None:
    before = output_of(running)
    time.sleep(0.2)
    after = output_of(running)
    assert after == before, f"process produced unexpected output: {after[len(before):]!r}"


def test_second_waits_for_first() -> None:
    second = spawn("second")
    try:
        read_until(second, "Second: début")
        assert_blocked(second)
        first = spawn("first")
        finish(first)
        output = finish(second)
        assert "first a terminé son attente" in output
        assert "Second: fin" in output
    finally:
        if second.poll() is None:
            second.kill()


def test_second_blocks_when_it_starts_first() -> None:
    second = spawn("second")
    try:
        read_until(second, "Second: début")
        assert_blocked(second)
    finally:
        if second.poll() is None:
            second.kill()


def test_two_process_rendezvous() -> None:
    first = spawn("rdv2", "0")
    second = spawn("rdv2", "1")
    try:
        read_until(first, "arrivé au rendez-vous")
        second_output = finish(second)
        first_output = finish(first)
        assert "rendez-vous atteint" in first_output
        assert "rendez-vous atteint" in second_output
    finally:
        for running in (first, second):
            if running.poll() is None:
                running.kill()


def test_three_process_rendezvous() -> None:
    processes = [spawn("rdv3", str(index)) for index in range(3)]
    try:
        read_until(processes[0], "arrivé au rendez-vous")
        outputs = [finish(process) for process in processes[1:]]
        outputs.append(finish(processes[0]))
        for output in outputs:
            assert "rendez-vous atteint" in output
    finally:
        for running in processes:
            if running.poll() is None:
                running.kill()


def test_emitter_waits_for_two_receivers() -> None:
    receiver_one = spawn("recepteur")
    try:
        read_until(receiver_one, "en attente")
        emitter = spawn("emetteur")
        read_until(emitter, "en attente d'un deuxième récepteur")
        receiver_two = spawn("recepteur")
        emitter_output = finish(emitter)
        first_output = finish(receiver_one)
        second_output = finish(receiver_two)
        assert "libéré, message" in emitter_output
        assert "libéré" in first_output
        assert "libéré" in second_output
    finally:
        if receiver_one.poll() is None:
            receiver_one.kill()


def test_emitter_that_arrives_first_is_released_by_two_receivers() -> None:
    emitter = spawn("emetteur")
    try:
        read_until(emitter, "en attente d'un deuxième récepteur")
        receiver_one = spawn("recepteur")
        read_until(receiver_one, "en attente")
        receiver_two = spawn("recepteur")
        emitter_output = finish(emitter)
        first_output = finish(receiver_one)
        second_output = finish(receiver_two)
        assert "libéré, message" in emitter_output
        assert "libéré" in first_output
        assert "libéré" in second_output
    finally:
        if emitter.poll() is None:
            emitter.kill()


def test_emitter_arriving_after_two_receivers_is_immediate() -> None:
    receiver_one = spawn("recepteur")
    receiver_two = spawn("recepteur")
    try:
        read_until(receiver_one, "en attente")
        read_until(receiver_two, "en attente")
        emitter = spawn("emetteur")
        emitter_output = finish(emitter)
        first_output = finish(receiver_one)
        second_output = finish(receiver_two)
        assert "deux récepteurs présents" in emitter_output
        assert "libéré" in first_output
        assert "libéré" in second_output
    finally:
        for running in (receiver_one, receiver_two):
            if running.poll() is None:
                running.kill()
