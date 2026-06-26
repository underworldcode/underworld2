# =============================================================================
# Underworld2 — multi-stage container build
#
# Stage 1 (builder): installs the full pixi 'default' environment (runtime +
#   build deps including compilers/cmake/swig), builds pip wheels for
#   lavavu, badlands, meshplex, and underworld2 itself, then installs the
#   lean 'runtime' pixi environment.
#
# Stage 2 (runtime): ubuntu:24.04 base with only the runtime conda env and
#   the pre-built wheels; no compilers shipped.
#
# Build:
#   podman build -f Containerfile -t underworld2:latest .   (preferred)
#   docker  build -f Containerfile -t underworld2:latest .
#
# For Binder (mybinder.org), .binder/Dockerfile is a symlink to this file
# so repo2docker picks it up automatically.
# =============================================================================

# ── Stage 1: builder ──────────────────────────────────────────────────────────
FROM ghcr.io/prefix-dev/pixi:latest AS builder

WORKDIR /app

# Copy manifest first so Docker can cache the solve step independently of
# source changes.  pixi.lock is optional on first run; pixi will create it.
COPY pixi.toml pixi.lock* ./

# Install the default environment (runtime + build deps, including compilers).
RUN pixi install --environment default

# Install pip packages that need compilers (lavavu, badlands, meshplex).
# --no-build-isolation ensures they see the conda env's PETSc, MPI, etc.
# meshplex is installed directly from GitHub (git URL kept here rather than
# in pixi.toml to keep the pixi solve portable across platforms).
RUN pixi run --environment default pip install --no-build-isolation \
    lavavu \
    badlands \
    "git+https://github.com/kinnala/meshplex.git"

# badlands uses the meson-python build backend.  Install its build tools
# explicitly (they are not project dependencies, just wheel-building
# infrastructure needed because we use --no-build-isolation).
RUN pixi run --environment default pip install meson meson-python

# Build redistributable wheels for those packages so the runtime stage can
# install them without compilers.
RUN mkdir /wheels && \
    pixi run --environment default pip wheel \
        --no-deps \
        --no-build-isolation \
        -w /wheels \
        lavavu \
        badlands \
        "git+https://github.com/kinnala/meshplex.git"

# Copy the full source tree and build the underworld2 wheel.
# --no-build-isolation lets CMake discover PETSc/MPI via PKG_CONFIG_PATH
# that pixi run sets from the active conda prefix.
COPY . .
RUN pixi run --environment default pip wheel \
        --no-deps \
        --no-build-isolation \
        -w /wheels \
        .

# Build the lean runtime environment (no compilers, no build tools).
RUN pixi install --environment runtime

# Generate a bash activation script for the runtime env so the environment
# is correctly activated without pixi being present in the final image.
RUN pixi shell-hook --environment runtime -s bash > /shell-hook.sh && \
    echo 'exec "$@"' >> /shell-hook.sh


# ── Stage 2: runtime ──────────────────────────────────────────────────────────
FROM ubuntu:24.04

ARG NB_USER=jovyan
ARG NB_UID=1000

# Create the notebook user that Binder / JupyterHub expects.
RUN useradd --create-home --shell /bin/bash --uid ${NB_UID} ${NB_USER}

WORKDIR /home/${NB_USER}

# Copy the runtime conda environment.  The path is kept identical to the
# builder so that RPATH $ORIGIN entries and any absolute-path conda metadata
# remain valid.
COPY --from=builder /app/.pixi/envs/runtime /app/.pixi/envs/runtime

# Install the pre-built wheels (lavavu, badlands, meshplex, underworld2)
# into the runtime conda environment.  No compilation occurs here.
COPY --from=builder /wheels /wheels
RUN /app/.pixi/envs/runtime/bin/pip install --no-build-isolation /wheels/*.whl && \
    rm -rf /wheels

# ── Environment activation ────────────────────────────────────────────────────
# Set the variables that conda/pixi activation normally exports so that
# every process in the container sees the runtime environment without
# needing to source an activation script.
#
# LD_LIBRARY_PATH: required on Linux for the dynamic linker to find conda-
#   provided shared libraries (PETSc, libxml2, MPI, …) because
#   CMAKE_INSTALL_RPATH_USE_LINK_PATH was removed from CMakeLists.txt to
#   avoid embedding absolute build-env paths in the .so RPATH.
#   On macOS the equivalent is DYLD_FALLBACK_LIBRARY_PATH, set by conda/pixi
#   activation; see docs/development/development_guidelines.md for details.
ENV PATH="/app/.pixi/envs/runtime/bin:${PATH}" \
    CONDA_PREFIX="/app/.pixi/envs/runtime" \
    CONDA_DEFAULT_ENV="runtime" \
    LD_LIBRARY_PATH="/app/.pixi/envs/runtime/lib"

# Keep the full activation script available for interactive / debugging use:
#   source /activate.sh
COPY --from=builder /shell-hook.sh /activate.sh

USER ${NB_USER}
