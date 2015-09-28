Note: you will need to set your PYTHONPATH environmental variable:

$ source updatePyPath.sh

also, on some system you may encounter this error:

python: symbol lookup error: /usr/lib/openmpi/lib/openmpi/mca_paffinity_linux.so: undefined symbol: mca_base_param_reg_int

in which case, try setting an LD_PRELOAD such as this:

export LD_PRELOAD=/usr/lib/openmpi/lib/libmpi.so

