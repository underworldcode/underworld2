#!/bin/sh

# Builds and watches the current directory
# Starts a web server at the _site directory

set -e
cd $(dirname "$0")/..
# cd notebooks

# We want to start the server from the _site directory
# where everything was built by the docker-site-builder script

jupyter-notebook --port=8888 --ip="*" --no-browser --NotebookApp.token="" \
                 --NotebookApp.allow_remote_access=True \
                 --NotebookApp.disable_check_xsrf=True \
                 --NotebookApp.allow_origin="*" \
                 # --NotebookApp.default_url="/tree/Underworld_Introduction.ipynb"
                 # --NotebookApp.default_url="$JUPYTERHUB_SERVICE_PREFIX/tree"

# Don't exit

while true; do
  sleep 600
done
