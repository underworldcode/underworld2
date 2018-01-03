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

# expose notebook port
EXPOSE 8888
# expose glucifer port
EXPOSE 9999

# add default user jovyan and change permissions on NB_WORK
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan

# copy this file over so that no password is required
COPY jupyter_notebook_config.json /home/$NB_USER/.jupyter/jupyter_notebook_config.json

# update all permissions for user
RUN chown -R $NB_USER:users /workspace $UW2_DIR /home/$NB_USER

# CHANGE USER
USER $NB_USER
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR

# setup symlink for terminal convenience 
RUN ln -s workspace /home/$NB_USER/

# create a volume
VOLUME $NB_WORK/user_data
WORKDIR /workspace

# launch notebook
CMD ["jupyter", "notebook", "--ip='*'", "--no-browser"]


