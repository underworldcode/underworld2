FROM underworldcode/underworld2_untested:dev
MAINTAINER mansourjohn@gmail.com

# set working directory to /root
WORKDIR /root/underworld2/utils

# run tests
RUN xvfbrun.sh ./run_tests.py ../docs/examples/1_*.ipynb ../docs/user_guide/*.ipynb ../docs/test/*

WORKDIR /workspace
