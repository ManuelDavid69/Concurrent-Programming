"""Graphics pipeline."""

from __future__ import annotations

from pathlib import Path

import pytest

from conftest import binary, run


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    assert data.startswith(b"P6")
    header, pixels = data.split(b"255\n", 1)
    fields = header.split()
    return int(fields[1]), int(fields[2]), pixels


@pytest.mark.parametrize(("raster", "shader"), [(1, 1), (2, 2), (4, 3)])
def test_pipeline_renders_the_scene(tmp_path: Path, raster: int, shader: int) -> None:
    output = tmp_path / "image.ppm"
    result = run([binary("pipeline", "pipeline"), str(raster), str(shader), output], timeout=60)
    assert result.returncode == 0
    assert "Pipeline terminé" in result.stdout
    width, height, pixels = read_ppm(output)
    assert (width, height) == (640, 480)
    assert len(pixels) == width * height * 3
    colors = {pixels[index : index + 3] for index in range(0, len(pixels), 3)}
    assert len(colors) >= 6
    background = b"\x12\x14\x1c"
    assert sum(
        1 for index in range(0, len(pixels), 3) if pixels[index : index + 3] != background
    ) > 50000


def test_pipeline_rejects_zero_threads() -> None:
    result = run([binary("pipeline", "pipeline"), "0", "2"], check=False)
    assert result.returncode == 1
