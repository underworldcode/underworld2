FROM underworldcode/underworld2:2.10.0b as underworld



ADD /docs/examples/05_Rayleigh_Taylor_modified.ipynb ./UNSW/

# Set the UW_MACHINE env variable for metrics
ENV UW_MACHINE binder