FROM underworldcode/underworld2:2.10.0b as underworld



ADD /UNSW/* ./examples/

# Set the UW_MACHINE env variable for metrics
ENV UW_MACHINE binder

USER root
RUN pip install hide_code
RUN jupyter nbextension install --py hide_code
RUN jupyter nbextension enable --py hide_code
RUN jupyter serverextension enable --py hide_code