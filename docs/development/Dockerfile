FROM underworldcode/docker-uwbase
MAINTAINER mansourjohn@gmail.com

# set working directory to /root
WORKDIR /root

# get underworld and compile
RUN git clone https://github.com/underworldcode/underworld2 && \
    cd underworld2/libUnderworld && \
    ./configure.py               && \
    ./scons.py                   && \
    cd libUnderworldPy           && \
    ./swigall.py                 && \
    cd ..                        && \
    ./scons.py


# setup environment
ENV PYTHONPATH $PYTHONPATH:/root/underworld2
ENV GIT_COMMIT `git --git-dir=/root/underworld2/.git rev-parse --short HEAD`

# add label to docker image
LABEL commit=$GIT_COMMIT

# Add a notebook profile.
RUN mkdir -p -m 700 /root/.jupyter/ && \
    echo "c.NotebookApp.ip = '*'" >> /root/.jupyter/jupyter_notebook_config.py

# setup spaces for notebooks
VOLUME /notebooks
WORKDIR /notebooks

# expose notebook port
EXPOSE 8888

# Install Tini.. this is required because CMD (below) doesn't play nice with notebooks for some reason: https://github.com/ipython/ipython/issues/7062, https://github.com/jupyter/notebook/issues/334
RUN curl -L https://github.com/krallin/tini/releases/download/v0.6.0/tini > tini && \
    echo "d5ed732199c36a1189320e6c4859f0169e950692f451c03e7854243b95f4234b *tini" | sha256sum -c - && \
    mv tini /usr/local/bin/tini && \
    chmod +x /usr/local/bin/tini
ENTRYPOINT ["tini", "--"]

# launch notebook
CMD ["jupyter", "notebook"]