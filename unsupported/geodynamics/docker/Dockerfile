FROM underworldcode/underworld2_untested:dev
MAINTAINER romain.beucher@unimelb.edu

RUN mkdir geodynamics
RUN rsync -av /root/underworld2/unsupported/geodynamics/examples /workspace/geodynamics
VOLUME /workspace/geodynamics/outputs
WORKDIR /workspace/geodynamics

# launch notebook
CMD ["jupyter", "notebook", " --no-browser"]


