Singularity Usage Instructions
==============================

The recommended method for running Underworld on Magnus is through the use of 
Singularity. Singularity allows docker images to be run on supporting HPC facilities.

Basic instructions for Singularity usage on Magnus is provided below. Note that by default, 
your home directory will not be mounted into your Singularity container. You should instead 
run from `/scratch` or `/group`, which are by default mounted. Please review the Singularity 
documentation at Pawsey for full usage instructions:
https://support.pawsey.org.au/documentation/display/US/Singularity


1. Load the Singularity module:
```shell
user@magnus-1:~$ module load singularity
```

2. Pull down the required image. Note that images are generally around 1GB, so you might 
consider storing them somewhere in `/group` or `/scratch`.
```shell
user@magnus-1:~$ singularity pull /WHERE/TO/STORE/YOUR/IMAGE/IMAGENAME.sif docker://underworldcode/underworld2:2.8.1b
```

3. Use the image. Note that this will normally be a command line in your
queue submission script. Don't forget to also load the Singularity module.
```shell
user@magnus-1:~$ srun --export=all singularity exec /WHERE/TO/STORE/YOUR/IMAGE/IMAGENAME.sif python3 YOURSCRIPT.py
```


