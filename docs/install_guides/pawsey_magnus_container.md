Singularity Usage Instructions
==============================

The recommended method for running Underworld on Magnus is through the use of 
Singularity. Singularity allows docker images to be run on supporting HPC facilities.

Basic instructions for Singularity usage on Magnus is provided below. Note that by default, 
your home directory will not be mounted into your Singularity container. You should instead 
run from `/scratch` or `/group`, which are by default mounted. Please review the Singularity 
documentation at Pawsey for full usage instructions:
https://support.pawsey.org.au/documentation/display/US/Containers


1. Load the Singularity module:
```shell
user@magnus-1:~$ module load singularity
```

2. Pull down the required image. Note that images are generally around 1GB, so you might 
   consider storing them somewhere in `/group` or `/scratch`.
```shell
user@magnus-1:~$ singularity pull /WHERE/TO/STORE/YOUR/IMAGE/IMAGENAME.sif docker://underworldcode/underworld2
```

3. Use the image. Note that this will normally be a command line in your
   queue submission script. Don't forget to also load the Singularity module.
   Note also the setting of the environment variable `UW_VIS_PORT` which is 
   required for correct vis operation.
   Note also that we currently need to set the `SINGULARITYENV_LD_LIBRARY_PATH`
   variable which prevents unwanted host libraries being loaded at runtime. 
```shell
user@magnus-1:~$ export SINGULARITYENV_LD_LIBRARY_PATH=/opt/cray/pe/mpt/7.7.0/gni/mpich-gnu-abi/4.9/lib:/opt/cray/xpmem/2.2.15-6.0.7.1_5.16__g7549d06.ari/lib64:/opt/cray/ugni/6.0.14.0-6.0.7.1_3.18__gea11d3d.ari/lib64:/opt/cray/udreg/2.3.2-6.0.7.1_5.18__g5196236.ari/lib64:/opt/cray/pe/pmi/5.0.13/lib64:/opt/cray/alps/6.6.43-6.0.7.1_5.54__ga796da32.ari/lib64:/opt/cray/wlm_detect/1.3.3-6.0.7.1_5.8__g7109084.ari/lib64:/usr/lib64
user@magnus-1:~$ export UW_VIS_PORT=0
user@magnus-1:~$ srun singularity exec /WHERE/TO/STORE/YOUR/IMAGE/IMAGENAME.sif python3 YOURSCRIPT.py
```

For interactive usage, you will start a Singularity shell:

1. First start an interactive session with however many tasks you require:
```shell
user@magnus-1:~$ salloc --ntasks=16
```

2. Once your session has begun, load the Singularity module:
```shell
user@magnus-1:~$ module load singularity
```

3. Launch the Singularity shell. Note the `--pty` required for pseudo terminal mode.
```shell
user@magnus-1:~$ export SINGULARITYENV_LD_LIBRARY_PATH=/opt/cray/pe/mpt/7.7.0/gni/mpich-gnu-abi/4.9/lib:/opt/cray/xpmem/2.2.15-6.0.7.1_5.16__g7549d06.ari/lib64:/opt/cray/ugni/6.0.14.0-6.0.7.1_3.18__gea11d3d.ari/lib64:/opt/cray/udreg/2.3.2-6.0.7.1_5.18__g5196236.ari/lib64:/opt/cray/pe/pmi/5.0.13/lib64:/opt/cray/alps/6.6.43-6.0.7.1_5.54__ga796da32.ari/lib64:/opt/cray/wlm_detect/1.3.3-6.0.7.1_5.8__g7109084.ari/lib64:/usr/lib64
user@magnus-1:~$ export UW_VIS_PORT=0
user@magnus-1:~$ srun --pty  singularity shell /WHERE/TO/STORE/YOUR/IMAGE/IMAGENAME.sif
```

