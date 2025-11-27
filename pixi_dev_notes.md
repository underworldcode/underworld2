## Pixi notes

Pixi is a new package manager for building Underworld2. It uses `conda-forge` and `PyPI` under the source packages. It installs them in consistent reproducible environments much like `conda`, but with the addedd benefit of lock files for deterministic builds. 

### Basic commands

`pixi install` to install an environment of uw2
`pixi run build` to build uw2 in the environment
`pixi run test` to run the uw2 tests in the pixi environment
`pixi shell` (in the same directory as `pixi.toml`) to enter into the environment.
