FROM underworldcode/underworld2_untested:latest
MAINTAINER https://github.com/underworldcode/

# set working directory to /opt
WORKDIR /opt/underworld2/utils

# run tests
RUN ./run_tests.py ../docs/examples/1_*.ipynb ../docs/user_guide/*.ipynb ../docs/test/*

WORKDIR /workspace
