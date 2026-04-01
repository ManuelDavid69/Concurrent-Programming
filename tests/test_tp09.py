"""TP 9 — signals."""

from __future__ import annotations

import os
import signal
import time

from conftest import binary, run
from process_utils import read_until, spawn, wait_for_exit


def test_exo1_intercepts_sigint() -> None:
    process = spawn([str(binary("tp09", "exo1"))])
    read_until(process, "En cours")
    process.send_signal(signal.SIGINT)
    code, output = wait_for_exit(process)
    assert code == 0
    assert "Signal SIGINT reçu" in output


def test_exo2_ignores_sigint_until_sigkill() -> None:
    process = spawn([str(binary("tp09", "exo2"))])
    read_until(process, "SIGINT est ignoré")
    process.send_signal(signal.SIGINT)
    time.sleep(0.2)
    assert process.poll() is None
    process.send_signal(signal.SIGKILL)
    process.wait()
    process.cleanup()


def test_exo3_stops_cleanly() -> None:
    process = spawn([str(binary("tp09", "exo3"))])
    read_until(process, "Traitement en cours")
    process.send_signal(signal.SIGINT)
    code, output = wait_for_exit(process)
    assert code == 0
    assert "Arrêt propre du programme" in output


def test_exo4_parent_kills_child() -> None:
    result = run([binary("tp09", "exo4")], timeout=20)
    assert result.returncode == 0
    assert result.stdout.count("Père: itération") == 3
    assert "envoi de SIGKILL" in result.stdout
    assert "le fils a été tué" in result.stdout


def test_exo5_child_survives_the_parent() -> None:
    process = spawn([str(binary("tp09", "exo5"))])
    output = read_until(process, "Fils pid=")
    child_pid = int(output.split("Fils pid=", 1)[1].split(":", 1)[0])
    code = process.wait(timeout=10)
    assert code == 0
    output = process.output()
    assert "le fils" in output and "continue tout seul" in output
    assert "Fils" in output

    # The child is still alive after the parent exits.
    os.kill(child_pid, 0)
    os.kill(child_pid, signal.SIGINT)
    time.sleep(0.2)
    output = process.output()
    assert "SIGINT intercepté" in output
    os.kill(child_pid, signal.SIGKILL)
    time.sleep(0.1)
    process.cleanup()


def test_exo6_uses_usr1_and_usr2() -> None:
    result = run([binary("tp09", "exo6")], timeout=20)
    assert result.returncode == 0
    assert result.stdout.count("SIGUSR1 reçu") == 2
    assert result.stdout.count("SIGUSR2 reçu") == 1
    assert "Fils: terminé" in result.stdout


def test_exo7_accepts_a_valid_integer() -> None:
    result = run(
        [binary("tp09", "exo7")],
        input_text="abc\n12\n",
        env={"PC_TIMEOUT_SECONDS": "2"},
    )
    assert result.returncode == 0
    assert "Ok merci !!" in result.stdout
    assert "Svp un entier" in result.stdout


def test_exo7_reports_a_timeout() -> None:
    process = spawn([str(binary("tp09", "exo7"))], env={"PC_TIMEOUT_SECONDS": "1"}, stdin_open=True)
    try:
        read_until(process, "Svp un entier")
        time.sleep(1.5)
        code = process.wait(timeout=10)
        output = process.output()
        assert code == 1
        assert "Trop tard !!" in output
    finally:
        process.kill()
