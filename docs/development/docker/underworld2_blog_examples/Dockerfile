FROM underworldcode/underworld2_untested:dev
MAINTAINER romain.beucher@unimelb.edu

RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
    gdal-bin \
    python-gdal


RUN pip install pandas
RUN rsync -av /root/underworld2/docs/BlogExamples /workspace
