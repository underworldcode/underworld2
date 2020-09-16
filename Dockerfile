FROM underworldcode/underworld2:2.10.0b as underworld



ADD /UNSW/* ./examples/

# Set the UW_MACHINE env variable for metrics
ENV UW_MACHINE binder


