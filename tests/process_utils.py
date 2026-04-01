"""Small helper for tests that drive a live process through signals or stdin."""

from __future__ import annotations

import os
import subprocess
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path


@dataclass
class Running:
    process: subprocess.Popen[bytes]
    stdout_path: Path
    stderr_path: Path

    def poll(self) -> int | None:
        return self.process.poll()

    def output(self) -> str:
        try:
            return self.stdout_path.read_text(encoding="utf-8", errors="replace")
        except FileNotFoundError:
            return ""

    def error(self) -> str:
        try:
            return self.stderr_path.read_text(encoding="utf-8", errors="replace")
        except FileNotFoundError:
            return ""

    def send_signal(self, signal_number: int) -> None:
        self.process.send_signal(signal_number)

    def wait(self, timeout: float = 10.0) -> int:
        try:
            self.process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            self.kill()
            raise
        return self.process.returncode

    def kill(self) -> None:
        if self.process.poll() is None:
            self.process.kill()
        self.process.wait()
        self.cleanup()

    def cleanup(self) -> None:
        self.stdout_path.unlink(missing_ok=True)
        self.stderr_path.unlink(missing_ok=True)


def spawn(
    arguments: list[str],
    *,
    env: dict[str, str] | None = None,
    stdin_open: bool = False,
) -> Running:
    environment = os.environ.copy()
    environment.setdefault("PC_SLEEP_SCALE", "0.05")
    environment.setdefault("LC_ALL", "C")
    if env:
        environment.update(env)
    stdout_fd, stdout_name = tempfile.mkstemp(prefix="test-out-")
    stderr_fd, stderr_name = tempfile.mkstemp(prefix="test-err-")
    stdout_file = os.fdopen(stdout_fd, "wb")
    stderr_file = os.fdopen(stderr_fd, "wb")
    try:
        process = subprocess.Popen(
            arguments,
            stdin=subprocess.PIPE if stdin_open else subprocess.DEVNULL,
            stdout=stdout_file,
            stderr=stderr_file,
            env=environment,
        )
    finally:
        stdout_file.close()
        stderr_file.close()
    return Running(process, Path(stdout_name), Path(stderr_name))


def read_until(running: Running, expected: str, timeout: float = 10.0) -> str:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        output = running.output()
        if expected in output:
            return output
        if running.poll() is not None:
            raise AssertionError(
                f"process exited before {expected!r}; output={output!r}; stderr={running.error()!r}"
            )
        time.sleep(0.02)
    raise AssertionError(f"did not see {expected!r}; output={running.output()!r}")


def wait_for_exit(running: Running, timeout: float = 10.0) -> tuple[int, str]:
    code = running.wait(timeout)
    output = running.output()
    running.cleanup()
    return code, output
