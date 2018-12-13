## Underworld Installation

### Docker

Docker is a type of lightweight virtualisation, and is the preferred method for Underworld usage on personal computers. New users may wish to use the [Kitematic GUI](https://github.com/docker/kitematic/releases) to download and run Underworld via Docker. Simply search for 'underworldcode/underworld2' within Kitematic, and then click 'CREATE' to launch a container. You will eventually wish to modify your container settings (again through Kitematic) to enable local folder volume mapping, which will allow you to access your local drives within your container.

For Linux users, and those who prefer the command line, the following minimal command should be sufficient to access the Underworld2 Jupyter Notebook examples:

```bash
   docker run -p 8888:8888 underworldcode/underworld2
```

Navigate to `localhost:8888` to see the notebooks.

### Native

For installation on HPC facilities and if you would like a local build, you will need to download, install and compile the Underworld code and relevant dependencies. A native build can be difficult
in a non-standard environment and we are currently working on HPC deployments of Docker containers.

For information on how to build, look on the underworld github pages under [docs/install_guides](https://github.com/underworldcode/underworld2/tree/master/docs/install_guides) and also see the [Underworld blog pages](http://www.underworldcode.org/pages/Blog/)
