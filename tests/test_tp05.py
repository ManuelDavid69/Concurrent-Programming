"""TP 5 — fork/exec, bounded parallelism and a mini build tool."""

from __future__ import annotations

import http.server
import os
import shutil
import stat
import threading
from pathlib import Path

import pytest

from conftest import ROOT, binary, run


def test_simultaneous_commands_are_all_started(tmp_path: Path) -> None:
    (tmp_path / "sample.txt").write_text("sample\n", encoding="ascii")
    result = run(
        [binary("tp05", "simultane")],
        env={"SIMULTANE_PATH": str(tmp_path)},
        timeout=30,
    )
    assert f"Lancé: du -sh {tmp_path}" in result.stdout
    assert "Lancé: ls -l" in result.stdout
    assert "Lancé: ps" in result.stdout
    assert result.returncode == 0


def test_successive_commands_are_ordered(tmp_path: Path) -> None:
    (tmp_path / "sample.txt").write_text("sample\n", encoding="ascii")
    result = run(
        [binary("tp05", "successif")],
        env={"SIMULTANE_PATH": str(tmp_path)},
        timeout=30,
    )
    lines = [line for line in result.stdout.splitlines() if line.startswith(("Lancé:", "Terminé:", "Échec:"))]
    assert lines[0].startswith(f"Lancé: du -sh {tmp_path}")
    assert lines[1] == f"Terminé: du -sh {tmp_path}"
    assert lines[2].startswith("Lancé: ls")
    assert lines[3] == "Terminé: ls -l"
    assert lines[4].startswith("Lancé: ps")
    assert lines[5] == "Terminé: ps"


def make_fake_convert(directory: Path) -> None:
    script = directory / "convert"
    script.write_text(
        "#!/bin/sh\n"
        "sleep 0.05\n"
        "printf 'P3\\n2 2\\n255\\n255 0 0 0 255 0 0 0 255 255 255 255\\n' > \"$4\"\n",
        encoding="ascii",
    )
    script.chmod(script.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def max_active(stdout: str) -> int:
    active = 0
    maximum = 0
    for line in stdout.splitlines():
        if line.startswith("Début conversion:"):
            active += 1
            maximum = max(maximum, active)
        elif line.startswith(("Fin conversion:", "Échec conversion:")):
            active -= 1
    return maximum


def test_thumbnails_are_bounded_to_eight_workers(tmp_path: Path) -> None:
    images = tmp_path / "images"
    thumbs = tmp_path / "thumbs"
    images.mkdir()
    fake_bin = tmp_path / "bin"
    fake_bin.mkdir()
    make_fake_convert(fake_bin)
    for index in range(12):
        (images / f"image-{index}.ppm").write_text(
            "P3\n2 2\n255\n255 0 0 0 255 0 0 0 255 255 255 255\n", encoding="ascii"
        )
    result = run(
        [binary("tp05", "miniature"), images, thumbs],
        env={"PATH": str(fake_bin) + os.pathsep + os.environ.get("PATH", "")},
        timeout=30,
    )
    assert "12 image(s) traitée(s)" in result.stdout
    assert max_active(result.stdout) <= 8
    assert len(list(thumbs.glob("*.ppm"))) == 12


def test_downloads_are_bounded_to_four_workers(tmp_path: Path) -> None:
    if shutil.which("wget") is None:
        pytest.skip("wget is not installed")
    served = tmp_path / "served"
    destination = tmp_path / "downloads"
    served.mkdir()
    for index in range(8):
        (served / f"file-{index}.txt").write_text(f"content {index}\n", encoding="ascii")

    class Handler(http.server.SimpleHTTPRequestHandler):
        def __init__(self, *args: object, **kwargs: object) -> None:
            super().__init__(*args, directory=str(served), **kwargs)

        def log_message(self, *_args: object) -> None:
            pass

    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        port = server.server_address[1]
        urls = [f"http://127.0.0.1:{port}/file-{index}.txt" for index in range(8)]
        result = run([binary("tp05", "telecharge"), *urls, destination], timeout=60)
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=5)

    assert result.returncode == 0
    active = 0
    maximum = 0
    for line in result.stdout.splitlines():
        if line.startswith("Début téléchargement:"):
            active += 1
            maximum = max(maximum, active)
        elif line.startswith(("Téléchargement terminé:", "Échec du téléchargement:")):
            active -= 1
    assert maximum <= 4
    assert sorted(path.name for path in destination.iterdir()) == [f"file-{index}.txt" for index in range(8)]
    assert (destination / "file-7.txt").read_text(encoding="ascii") == "content 7\n"


def test_mini_make_compiles_and_links(tmp_path: Path) -> None:
    success = ROOT / "tp05" / "samples" / "success"
    executable = tmp_path / "demo"
    result = run(
        [
            binary("tp05", "make"),
            success / "main.c",
            success / "mathutils.c",
            success / "message.c",
            executable,
        ],
        cwd=tmp_path,
        timeout=60,
    )
    assert result.returncode == 0
    assert executable.exists()
    output = run([executable], cwd=tmp_path)
    assert output.stdout == "2 + 3 = 5\nBonjour, CPE Lyon!\n"


def test_mini_make_stops_after_a_compilation_error(tmp_path: Path) -> None:
    failure = ROOT / "tp05" / "samples" / "failure"
    executable = tmp_path / "broken"
    result = run(
        [binary("tp05", "make"), failure / "bad_syntax.c", failure / "bad_include.c", executable],
        cwd=tmp_path,
        check=False,
        timeout=60,
    )
    assert result.returncode == 1
    assert "Erreur de compilation" in result.stdout
    assert not executable.exists()


def test_mini_make_reports_a_link_error(tmp_path: Path) -> None:
    failure = ROOT / "tp05" / "samples" / "failure"
    executable = tmp_path / "missing"
    result = run(
        [binary("tp05", "make"), failure / "missing_symbol.c", executable],
        cwd=tmp_path,
        check=False,
        timeout=60,
    )
    assert result.returncode == 1
    assert "édition de liens échouée" in result.stderr
    assert not executable.exists()
