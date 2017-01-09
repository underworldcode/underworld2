glucifer module
===============

The glucifer module provides visualisation algorithms for Underworld. 

Visualisation data is generated in parallel, with each processes generating the 
necessary data for its part of the domain. This data is written into a data file. 

Actual rendering is performed in serial using the LavaVu rendering engine.

glucifer provides many flexible rendering options, including client-server based
operation for remote usage. Users may choose to renderer outputs to raster images, 
or save a database file for later rendering. For those working in the Jupyter 
environment, glucifer will inline rendered images or even interactive webgl 
frames (still experimental).   



Module Summary
--------------
submodules:
~~~~~~~~~~~

.. toctree::
    :maxdepth: 1

    glucifer.objects

classes:
~~~~~~~~

.. autosummary::
    :nosignatures:

    glucifer.Viewer
    glucifer.Store
    glucifer.Figure

Module Details
--------------
classes:
~~~~~~~~
.. autoclass:: glucifer.Viewer
    :members:
    :show-inheritance:
.. autoclass:: glucifer.Store
    :members:
    :show-inheritance:
.. autoclass:: glucifer.Figure
    :members:
    :show-inheritance:

