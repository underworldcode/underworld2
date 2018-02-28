FROM underworldcode/underworld2_untested:dev
MAINTAINER romain.beucher@unimelb.edu

USER root

WORKDIR /opt
RUN git clone https://github.com/rbeucher/UWGeodynamics.git
RUN pip install -e /opt/UWGeodynamics
RUN mkdir /workspace/UWGeodynamics
RUN rsync -av /opt/UWGeodynamics/examples/* /workspace/UWGeodynamics/examples/
RUN rsync -av /opt/UWGeodynamics/tutorials/* /workspace/UWGeodynamics/tutorials/
RUN rsync -av /opt/UWGeodynamics/manual/* /workspace/UWGeodynamics/manual/

# update all permissions for jovyan user
ENV NB_USER jovyan
RUN chown -R $NB_USER:users /workspace $UW2_DIR /home/$NB_USER

# change user and update pythonpath
USER $NB_USER
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR

# move back to workspace directory
WORKDIR /workspace

# launch notebook
CMD ["jupyter", "notebook", "--ip='*'", "--no-browser"]


