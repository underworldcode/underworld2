FROM underworldcode/underworld2-dev:dev
MAINTAINER mansourjohn@gmail.com

# set working directory to /root
WORKDIR /root/underworld2/utils

# run tests
RUN xvfbrun.sh ./run_tests.py ../docs/examples/1_* ../docs/user_guide/* ../docs/test/*

WORKDIR /workspace
