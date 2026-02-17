"""TP 3 — PPM blur and digicode state machines."""

from __future__ import annotations

from pathlib import Path

import pytest

from conftest import binary, run


def write_ppm(path: Path, width: int, height: int, pixels: list[tuple[int, int, int]], ascii_format: bool = True) -> None:
    assert len(pixels) == width * height
    lines = ["P3" if ascii_format else "P6", f"{width} {height}", "255"]
    if ascii_format:
        for red, green, blue in pixels:
            lines.append(f"{red} {green} {blue}")
        path.write_text("\n".join(lines) + "\n", encoding="ascii")
    else:
        header = "\n".join(lines) + "\n"
        body = bytes(channel for pixel in pixels for channel in pixel)
        path.write_bytes(header.encode("ascii") + body)


def read_ppm(path: Path) -> tuple[int, int, list[tuple[int, int, int]]]:
    data = path.read_bytes()
    assert data.startswith(b"P6")
    tokens: list[bytes] = []
    index = 0
    # Parse the four header tokens (P6, width, height, maxval).
    while len(tokens) < 4:
        while index < len(data) and data[index : index + 1].isspace():
            index += 1
        if data[index : index + 1] == b"#":
            while index < len(data) and data[index : index + 1] != b"\n":
                index += 1
            continue
        start = index
        while index < len(data) and not data[index : index + 1].isspace():
            index += 1
        tokens.append(data[start:index])
    index += 1  # exactly one whitespace after maxval
    width = int(tokens[1])
    height = int(tokens[2])
    assert int(tokens[3]) == 255
    raw = data[index : index + width * height * 3]
    assert len(raw) == width * height * 3
    pixels = [tuple(raw[offset : offset + 3]) for offset in range(0, len(raw), 3)]
    return width, height, pixels


def test_blur_uniform_image_is_unchanged(tmp_path: Path) -> None:
    source = tmp_path / "uniform.ppm"
    output = tmp_path / "out.ppm"
    write_ppm(source, 3, 3, [(120, 80, 40)] * 9)
    result = run([binary("tp03", "flou"), source, output])
    assert "Image 3x3 traitée" in result.stdout
    width, height, pixels = read_ppm(output)
    assert (width, height) == (3, 3)
    assert pixels == [(120, 80, 40)] * 9


def test_blur_uses_kernel_crop_on_the_border(tmp_path: Path) -> None:
    source = tmp_path / "corner.ppm"
    output = tmp_path / "out.ppm"
    # Only the top-left pixel is white; it has four in-bounds neighbours
    # (itself, right, down, down-right), so the blurred corner is 255/4 = 63.
    pixels = [(0, 0, 0)] * 9
    pixels[0] = (255, 255, 255)
    write_ppm(source, 3, 3, pixels)
    run([binary("tp03", "flou"), source, output])
    _, _, blurred = read_ppm(output)
    assert blurred[0] == (63, 63, 63)
    assert blurred[4] == (28, 28, 28)


def test_blur_accepts_binary_ppm(tmp_path: Path) -> None:
    source = tmp_path / "binary.ppm"
    output = tmp_path / "out.ppm"
    write_ppm(source, 2, 2, [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 255)], ascii_format=False)
    run([binary("tp03", "flou"), source, output])
    width, height, _ = read_ppm(output)
    assert (width, height) == (2, 2)


def test_blur_of_the_committed_sample(tmp_path: Path) -> None:
    output = tmp_path / "sample-out.ppm"
    result = run([binary("tp03", "flou"), Path("tp03/sample.ppm"), output])
    assert "Image 6x5 traitée" in result.stdout
    assert read_ppm(output)[:2] == (6, 5)


@pytest.mark.parametrize(
    ("code", "typed", "expected"),
    [
        ("5742", "1235742", "Porte ouverte"),
        ("5742", "57425742", "Porte ouverte\nPorte ouverte"),
        ("5742", "5743", "Aucune séquence correcte"),
        ("5742", "5x7x4x2", "Porte ouverte"),
        ("5551", "125551", "Porte ouverte"),
        ("5551", "5555", "Aucune séquence correcte"),
        ("8267B", "8267B", "Code valide"),
        ("8267B", "78267B", "Code invalide"),
        ("8267B", "7A8267B", "Code invalide\nCode valide"),
        ("8267B", "8267A", "Code invalide"),
    ],
)
def test_digicode(code: str, typed: str, expected: str) -> None:
    result = run([binary("tp03", "digicode"), code], input_text=typed)
    assert expected in result.stdout


def test_digicode_trace_reports_states() -> None:
    result = run([binary("tp03", "digicode"), "5742", "--trace"], input_text="5742")
    assert "'5' -> état 1" in result.stdout
    assert "'2' -> état 0" in result.stdout
    assert "Porte ouverte" in result.stdout


def test_transition_tables_are_printed() -> None:
    result = run([binary("tp03", "table")])
    assert "Automate pour 5742" in result.stdout
    assert "Automate pour 5551" in result.stdout
    assert "8267B" in result.stdout
    assert result.stdout.count("ouvre") >= 2
