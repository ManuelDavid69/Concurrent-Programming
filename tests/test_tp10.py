"""TP 10 — shared memory, barriers, producer/consumer and exchange."""

from __future__ import annotations

import re
import subprocess
import time
from collections.abc import Iterator

import pytest

from conftest import binary, run


@pytest.fixture(autouse=True)
def clean_resources() -> Iterator[None]:
    run([binary("tp10", "clean")], check=False)
    yield
    run([binary("tp10", "clean")], check=False)


def initialize(capacity: int = 5, producers: int = 1) -> None:
    run([binary("tp10", "init"), str(capacity), str(producers)])


def spawn(name: str, *arguments: str) -> subprocess.Popen[str]:
    return subprocess.Popen(
        [str(binary("tp10", name)), *arguments],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env={"PC_SLEEP_SCALE": "0.05", "LC_ALL": "C", "PATH": "/usr/bin:/bin"},
    )


def finish(process: subprocess.Popen[str]) -> str:
    output, error = process.communicate(timeout=20)
    assert process.returncode == 0, error
    return output


@pytest.mark.parametrize("parties", [1, 2, 3, 5])
def test_n_way_rendezvous(parties: int) -> None:
    initialize()
    processes = [spawn("rdv_n", str(parties)) for _ in range(parties - 1)]
    try:
        time.sleep(0.2)
        assert all(process.poll() is None for process in processes)
        processes.append(spawn("rdv_n", str(parties)))
        outputs = [finish(process) for process in processes]
        for output in outputs:
            assert "rendez-vous atteint" in output
    finally:
        for process in processes:
            if process.poll() is None:
                process.kill()


def test_producer_consumer_is_fifo_with_one_producer() -> None:
    initialize(capacity=2, producers=1)
    producer = spawn("prod", "10")
    consumer = spawn("conso")
    producer_output = finish(producer)
    consumer_output = finish(consumer)
    produced = [int(match) for match in re.findall(r"dépose (\d+)", producer_output)]
    consumed = [int(match) for match in re.findall(r"récupère (\d+)", consumer_output)]
    assert consumed == produced
    assert len(consumed) == 10
    assert "terminé (10 objets)" in consumer_output


def test_several_producers_and_consumers() -> None:
    initialize(capacity=4, producers=2)
    producers = [spawn("prod", "12"), spawn("prod", "12")]
    consumers = [spawn("conso"), spawn("conso")]
    produced_values: list[int] = []
    for producer in producers:
        produced_values.extend(
            int(match) for match in re.findall(r"dépose (\d+)", finish(producer))
        )
    consumed_values: list[int] = []
    for consumer in consumers:
        consumed_values.extend(
            int(match) for match in re.findall(r"récupère (\d+)", finish(consumer))
        )
    assert sorted(produced_values) == sorted(consumed_values)
    assert len(consumed_values) == 24


def test_value_exchange_separates_the_tables() -> None:
    initialize()
    p1 = spawn("p1")
    p2 = spawn("p2")
    p0 = spawn("p0")
    first = finish(p1)
    second = finish(p2)
    controller = finish(p0)
    table_one = [int(value) for value in re.search(r"P1:(.*)", first).group(1).split()]
    table_two = [int(value) for value in re.search(r"P2:(.*)", second).group(1).split()]
    assert max(table_one) <= min(table_two)
    assert sorted(table_one + table_two) == sorted(
        list(range(1, 9)) + list(range(20, 91, 10))
    )
    assert "8 échange(s)" in controller


def test_rendezvous_requires_all_participants() -> None:
    initialize()
    first = spawn("rdv_n", "2")
    try:
        time.sleep(0.3)
        assert first.poll() is None
        second = spawn("rdv_n", "2")
        assert "rendez-vous atteint" in finish(first)
        assert "rendez-vous atteint" in finish(second)
    finally:
        if first.poll() is None:
            first.kill()
