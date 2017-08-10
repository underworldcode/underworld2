FROM underworldcode/underworld2_untested:dev
MAINTAINER romain.beucher@unimelb.edu

RUN mkdir geodynamics
RUN rsync -av /root/underworld2/unsupported/geodynamics/examples/ /workspace/geodynamics
VOLUME /workspace/geodynamics/outputs
WORKDIR /workspace/geodynamics

# expose notebook port
EXPOSE 8888
# expose glucifer port
EXPOSE 9999
# launch notebook
CMD ["jupyter", "notebook", " --no-browser"]


