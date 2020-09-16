FROM underworldcode/underworld2:2.10.0b as underworld



ADD /UNSW/* ./examples/

# Set the UW_MACHINE env variable for metrics
ENV UW_MACHINE binder

USER root
RUN pip install hide_code


RUN jupyter nbextension install hide_code --py --sys-prefix

RUN jupyter-nbextension enable hide_code --py --sys-prefix && \
    jupyter-serverextension enable hide_code --py --sys-prefix
USER $NB_USER
