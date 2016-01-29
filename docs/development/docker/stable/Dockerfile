FROM underworldcode/underworld2-dev
MAINTAINER mansourjohn@gmail.com

# set working directory to /root
WORKDIR /root/underworld2/utils

# run tests
RUN xvfbrun.sh ./run_tests.py ../docs/examples/* ../docs/user_guide/* ../docs/publications/* ../docs/test/*

WORKDIR /workspace
