## Creating a spack environment for UW2

These are rough instructions for building and using a spack environment with Underworld2.
This is experimental and may not work. If errors occur please raise a GitHub issue [here](https://github.com/underworldcode/underworld2/issues)

### 1. Download spack

Get a release version.
see further notes [here](https://spack-tutorial.readthedocs.io/en/latest/tutorial_basics.html)

```bash
git clone --branch=release/latest https://github.com/spack/spack.git ~/spack
cd ~/spack
. share/spack/setup-env.sh
```

Now you have spack available, setup-env.sh will need to be run in each new terminal.

### 2. Create and activate the spack environment

Using the file `spack.yaml` (in this directory) we will make a spack environment called `uw2-env`
This is similar to a conda environment only with spack.

```bash
spack env create uw2-env spack.yaml
spack env activate -p uw2-env
```

### Using / installing dependency software for UW2

`spack env status`            # check environment status
`spack find`                  # to see what specs are defined
`spack concretize -f --fresh` # to 'align' all specs together
`spack install`               # to download and install specs

`concretize` and `install` will naturally take a while. If you wish to add dependencies one can use `spack add` or `pip install`.

### Install underworld 2

```bash
cd /path/to/underworld2
pip install .
```
