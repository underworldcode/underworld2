TACC/Stampede2 Singularity Usage
================================

The `Dockerfile` here builds an Underworld Docker image based upon the TACC image 
which provides an optimal MPI configuration (MPI library and associated software 
for Infiniband hardware). 

Note that the standard Underworld image also operates correctly on Stampede2, 
although will not utilise Infiniband interconnects.


Pulling down the required image
-------------------------------

You will use Singularity on Stampede2 to pull down the required image via the 
`singularity pull` command. Note that `singularity` will not operate on the login 
node. You will need to start an interactive session (`idev`), load the Singularity 
module, and then you can pull down the required image: 

```bash
$ idev
$ module load tacc-singularity/2.6.0
$ singularity pull docker://underworldcode/underworld2:2.7.1b_stampede2
```

or for the standard Underworld image:

```bash
$ singularity pull docker://underworldcode/underworld2:2.7.1b
```

Available Underworld images are listed on [our DockerHub page](https://cloud.docker.com/u/underworldcode/repository/docker/underworldcode/underworld2/tags). Note that
only Underworld 2.7+ images will operate correctly on Stampede2.

The location of the pulled down image will be reported at the conclusion of the `pull`
command. Generally images are recorded to a `singularity_cache` directory which is 
set by the `SINGULARITY_CACHEDIR` environment variable:
```bash
$ echo ${SINGULARITY_CACHEDIR}
```

Using an image to run your model
--------------------------------

Once you have obtained the required image, you are ready to run your model via the
`singularity exec` command:

```bash
$ mpirun singularity exec $SINGULARITY_CACHEDIR/underworld2-2.7.1b_stampede2 python YourScript.py
```

The usual queued job submission instructions apply.

Note finally that you can actually point `singularity exec` directly at a docker image
and it will pull it down automatically if it does not exist locally:

```bash
$ singularity exec docker://underworldcode/underworld2:2.7.1b python YourScript.py
```

However, this will perform the image extraction process (~120 seconds) each time you
execute the command, so it is recommended that you explicitly pull down the required
image instead of relying on the automated process. 
