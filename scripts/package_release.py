#!/usr/bin/env python3
"""Pack the built game with its assets and non-system shared libraries."""

import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"
DIST = ROOT / "dist"

LINUX_SKIP_PREFIXES = (
    "libc.so",
    "libm.so",
    "libdl.so",
    "libpthread.so",
    "librt.so",
    "libresolv.so",
    "libutil.so",
    "ld-linux",
    "libGL.so",
    "libGLX.so",
    "libGLdispatch.so",
    "libGLU.so",
    "libOpenGL.so",
    "libX11.so",
    "libXext.so",
    "libxcb.so",
    "libXau.so",
    "libXdmcp.so",
    "libstdc++.so",
    "libgcc_s.so",
    "libdrm.so",
)


def run(command):
    subprocess.run(command, check=True)


def host_os():
    name = platform.system().lower()
    if name.startswith("mingw") or name.startswith("msys") or os.name == "nt":
        return "windows"
    if name == "darwin":
        return "macos"
    if name == "linux":
        return "linux"
    sys.exit(f"unsupported system: {platform.system()}")


def host_arch():
    machine = platform.machine().lower()
    if machine in ("amd64", "x86_64"):
        return "x64"
    if machine in ("arm64", "aarch64"):
        return "arm64"
    return machine


def version_name():
    if os.environ.get("GITHUB_REF_TYPE") == "tag":
        return os.environ.get("GITHUB_REF_NAME", "dev")
    return "dev"


def binary_path(system):
    if system == "windows":
        return ROOT / "build" / "windows" / "excitebike.exe"
    return ROOT / "build" / system / "excitebike"


def copy_assets(destination):
    def ignore(directory, names):
        if Path(directory) == ASSETS and "source" in names:
            return {"source"}
        return set()

    shutil.copytree(ASSETS, destination / "assets", ignore=ignore)


def macho_deps(path):
    output = subprocess.check_output(["otool", "-L", str(path)], text=True)
    deps = []
    for line in output.splitlines()[1:]:
        line = line.strip()
        if not line:
            continue
        deps.append(line.split(" (", 1)[0].strip())
    return deps


def is_system_macho(dep):
    return dep.startswith(("/usr/lib/", "/System/"))


def resolve_macho(current, dep):
    if dep.startswith("@loader_path/"):
        return Path(current).resolve().parent / dep.removeprefix("@loader_path/")
    if dep.startswith("@rpath/"):
        candidate = Path(current).resolve().parent / dep.removeprefix("@rpath/")
        return candidate if candidate.exists() else None
    if dep.startswith("@"):
        return None
    return Path(dep)


def bundle_macos(binary, stage):
    libs = stage / "libs"
    libs.mkdir()
    pending = [binary]
    queued = {binary.resolve()}

    while pending:
        current = pending.pop()
        for index, dep in enumerate(macho_deps(current)):
            if is_system_macho(dep) or dep.startswith("@"):
                continue
            source = resolve_macho(current, dep)
            if source is None or not source.exists():
                sys.exit(f"missing library {dep} required by {current}")
            bundled_name = source.name
            dest = libs / bundled_name
            new_ref = f"@executable_path/libs/{bundled_name}"
            if current != binary and index == 0:
                run(["install_name_tool", "-id", new_ref, str(current)])
                continue
            if not dest.exists():
                shutil.copy2(source, dest)
                dest.chmod(0o755)
                run(["codesign", "--remove-signature", str(dest)])
                if dest.resolve() not in queued:
                    queued.add(dest.resolve())
                    pending.append(dest)
            run(["install_name_tool", "-change", dep, new_ref, str(current)])

    bundle_sdl3(stage, libs)
    for path in [binary, *libs.iterdir()]:
        if path.is_file():
            run(["codesign", "--force", "--sign", "-", str(path)])


def bundle_sdl3(stage, libs):
    sdl2_libs = list(libs.glob("libSDL2*.dylib"))
    if not any(b"libSDL3.dylib" in path.read_bytes() for path in sdl2_libs):
        return
    source = find_sdl3()
    if source is None:
        sys.exit("SDL2 compatibility library needs libSDL3.dylib, but SDL3 was not found")
    for directory, install_id in (
        (stage, "@executable_path/libSDL3.dylib"),
        (libs, "@executable_path/libs/libSDL3.dylib"),
    ):
        dest = directory / "libSDL3.dylib"
        shutil.copy2(source, dest)
        dest.chmod(0o755)
        run(["codesign", "--remove-signature", str(dest)])
        run(["install_name_tool", "-id", install_id, str(dest)])
        run(["codesign", "--force", "--sign", "-", str(dest)])


def find_sdl3():
    candidates = []
    brew = shutil.which("brew")
    if brew:
        prefix = subprocess.run([brew, "--prefix", "sdl3"], text=True, capture_output=True)
        if prefix.returncode == 0:
            candidates.append(Path(prefix.stdout.strip()) / "lib" / "libSDL3.0.dylib")
    candidates.extend([
        Path("/opt/homebrew/opt/sdl3/lib/libSDL3.0.dylib"),
        Path("/usr/local/opt/sdl3/lib/libSDL3.0.dylib"),
    ])
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def ldd_deps(path):
    output = subprocess.check_output(["ldd", str(path)], text=True, stderr=subprocess.STDOUT)
    deps = []
    for line in output.splitlines():
        if "=>" not in line:
            continue
        soname, rest = line.split("=>", 1)
        soname = soname.strip()
        location = rest.strip().split()[0] if rest.strip() else ""
        if location.startswith("/"):
            deps.append((soname, Path(location)))
    return deps


def skip_linux(soname):
    return soname.startswith(LINUX_SKIP_PREFIXES)


def bundle_linux(binary, stage):
    libs = stage / "libs"
    libs.mkdir()
    pending = [binary]
    seen = set()
    while pending:
        current = pending.pop()
        for soname, source in ldd_deps(current):
            if skip_linux(soname) or not source.exists():
                continue
            dest = libs / soname
            if dest in seen:
                continue
            seen.add(dest)
            shutil.copy2(source, dest)
            dest.chmod(0o755)
            pending.append(dest)
    run(["patchelf", "--set-rpath", "$ORIGIN/libs", str(binary)])
    for lib in libs.iterdir():
        if lib.is_file():
            run(["patchelf", "--set-rpath", "$ORIGIN", str(lib)])


def is_mingw_runtime(path):
    text = str(path).replace("\\", "/").lower()
    return any(part in text for part in ("/ucrt64/", "/mingw64/", "/clang64/"))


def bundle_windows(binary, stage):
    pending = [binary]
    seen = {binary.resolve()}
    while pending:
        current = pending.pop()
        for _soname, source in ldd_deps(current):
            if not is_mingw_runtime(source):
                continue
            dest = stage / source.name
            if dest.resolve() in seen:
                continue
            shutil.copy2(source, dest)
            seen.add(dest.resolve())
            pending.append(dest)


def main():
    system = host_os()
    binary_src = binary_path(system)
    if not binary_src.exists():
        sys.exit(f"binary not found: {binary_src}")
    if not ASSETS.exists():
        sys.exit(f"assets not found: {ASSETS}")

    folder = f"excitebike-{version_name()}-{system}-{host_arch()}"
    stage = DIST / folder
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)

    binary = stage / binary_src.name
    shutil.copy2(binary_src, binary)
    binary.chmod(0o755)
    copy_assets(stage)

    if system == "macos":
        run(["codesign", "--remove-signature", str(binary)])
        bundle_macos(binary, stage)
    elif system == "linux":
        bundle_linux(binary, stage)
    else:
        bundle_windows(binary, stage)

    archive_format = "zip" if system == "windows" else "gztar"
    archive = shutil.make_archive(str(DIST / folder), archive_format, DIST, folder)
    shutil.rmtree(stage)
    print(archive)


if __name__ == "__main__":
    main()
