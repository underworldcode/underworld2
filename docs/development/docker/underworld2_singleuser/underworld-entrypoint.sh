#!/bin/bash
# Copyright (c) CSIRO Data61
# Distributed under the terms of the Modified BSD License.

set -e

# If we're being launched by jupyterhub then initialise the user's environment
if [[ ! -z "${JUPYTERHUB_API_TOKEN}" ]]
then
    # Use the $NB_USER's home workspace by default, unless a notebook directory
    # was passed.
    : ${NOTEBOOK_DIR:=/home/$NB_USER}

    # Make sure the notebook dir exists and is a directory.
    if [ -d "${NOTEBOOK_DIR}" ]; then

        # Copy underworld notebooks into user's workspace, making sure not to
        # overwrite any existing copies which the user may have modified and saved
        # on a previous run.
        rsync -au /workspace/.ipython /workspace/.jupyter /workspace/* "${NOTEBOOK_DIR}/"

        # Fix links in static html files to account for the jupyterhub proxy
        # serving content on a prefix path.
        ( shopt -s globstar dotglob
          sed -i -- "s/\\(href\\|src\\)=\"\\/\\(tree\\|files\\|terminals\\|edit\\)\\([^\"]*\\)\"/\\1=\"${JUPYTERHUB_SERVICE_PREFIX//\//\\\/}\\2\\3\"/g" "${NOTEBOOK_DIR}"/**/*.html
        )

    else
        echo Unable to copy notebooks from workspace
        echo "${NOTEBOOK_DIR}" is not a directory!
    fi
fi

# No longer required, since glucifer can invoke xvfb directly
# # Wrap the provided command with xvfb-run so we can do viz
# rm -f /tmp/.X99-lock && xvfb-run -s '-screen 0 1600x1200x16' $@

$@
