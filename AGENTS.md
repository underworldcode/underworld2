# AGENTS.md — Underworld2

## Setup & build

- **Python**: >=3.9,<3.13 (conda build pins to <3.12). **numpy <2** required.
- **Env manager**: pixi (`pixi.toml` at root). Commands: `pixi install` → `pixi run build` → `pixi run test`.
- **C/C++ build** requires: PETSc (found via `pkg-config`; set `PETSC_DIR`), MPI compilers (`MPICC`, `MPICXX`, `MPIF90`), SWIG >=4.0, CMake >=4.0, Ninja, libxml2, MPI-enabled h5py (`h5py=*=mpi*`).
- **Install**: `pip install .` or `pip install . --no-build-isolation` (CI style). Pass CMake options via `python setup.py build_ext -D"FOO=BAR"`.
- **Clean**: `bash clean.sh` (removes build artifacts, compiled `.so`, `_uwid.py`, caches).

## Tests

- **Run all**: `pytest ./docs/pytests/tests.py ./docs/pytests/test_examples.py ./docs/pytests/test_user_guide.py ./docs/pytests/test_UWGeo_examples.py ./docs/pytests/test_UWGeo_tutorial11.py`
- **CI subset** (fastest check): `pytest ./docs/pytests/tests.py ./docs/pytests/test_examples.py ./docs/pytests/test_UWGeo_examples.py`
- All test runners use `subprocess` to execute `.py` or `.ipynb` files. Notebooks are run via `pytest --nbmake`.
- Test notebooks live in `docs/test/`, `docs/examples/`, `docs/user_guide/`, `docs/UWGeodynamics/examples/`, `docs/UWGeodynamics/tutorials/`.

## Package structure

| Path | Role |
|---|---|
| `src/underworld/` | Main Python package (set `package_dir={"":"src"}` in setup.py) |
| `src/underworld/libUnderworld/` | C/C++ core + SWIG wrappers, built via CMake |
| `src/underworld/UWGeodynamics/` | High-level geodynamics API |
| `src/underworld/visualisation/` | LavaVu-based viz |

## Critical quirks

1. **dlopen flags**: `__init__.py` sets `RTLD_GLOBAL` before importing `.so` modules. This must happen early — do not reorder imports.
2. **h5py first**: h5py is imported before anything else to avoid HDF5 library conflicts with PETSc.
3. **`_uwid.py`**: Auto-generated on first install if missing (random UUID for usage metrics). Safe to delete/recreate.
4. **Usage metrics**: Opt out via `UW_NO_USAGE_METRICS` env var. In MPI, only rank 0 sends data.
5. **MPI exception handling**: In multi-process runs, exceptions are caught and broadcast via barrier. Set `UW_ALL_MESSAGES` to see all ranks' tracebacks.
6. **PRs**: Target `development` branch. Must update `CHANGELOG.md` and docstrings. See `CONTRIBUTING.rst`.

## Docs

- ReadTheDocs: https://underworld2.readthedocs.io (Sphinx config at `docs/development/docs_generator/conf.py`).
- Build docs via `docs/development/docs_generator/requirements.txt` deps.
