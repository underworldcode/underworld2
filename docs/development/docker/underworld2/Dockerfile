#FROM underworldcode/underworld2_untested:dev
FROM special
MAINTAINER https://github.com/underworldcode/

# set working directory to /opt
WORKDIR $UW2_DIR/utils

# run tests
RUN ./run_tests.py ../docs/examples/1_*.ipynb ../docs/user_guide/*.ipynb ../docs/test/*

WORKDIR /workspace
