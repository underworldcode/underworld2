.. underworld documentation master file, created by
   sphinx-quickstart on Mon Nov 21 16:44:00 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Underworld
==========

.. image:: ./images/Montage.png

`Underworld <http://www.underworldcode.org>`_ is a Python friendly version of the Underworld code which provides a programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. This gives signficant advantages to the user, with access to the power of Python libraries for setup of complex problems, analysis at runtime, problem steering, and multi physics coupling. While Underworld2 embraces Jupyter Notebooks as the preferred modelling environment, only standard Python is required.

The Underworld2 development team is based in Melbourne, Australia at the University of Melbourne and at Monash University led by Louis Moresi. We would like to acknowledge AuScope Simulation, Analysis and Modelling for providing long term funding which has made the project possible. Additional funding for specific improvements and additional functionality has come from the `Australian Research Council <http://www.arc.gov.au>`_. The Python toolkit was funded by the NeCTAR eresearch_tools program. Underworld was originally developed in collaboration with the Victorian Partnership for Advanced Computing.


Privacy
~~~~~~~

Note that basic usage metrics are dispatched when you use Underworld. We do this to help
assess the usage of our code which is important in justifying our funding. To opt out, set the UW_NO_USAGE_METRICS environment variable. See `PRIVACY.md <https://github.com/underworldcode/underworld2/blob/master/PRIVACY.md>`_ for full details.


Bedtime reading
~~~~~~~~~~~~~~~

These papers explain the theory and implementation for Underworld. The code itself can also be cited via the zenodo DOI. There is a master DOI for all releases and releases after 2.6.0 are automatically given a DOI under the master. If you are using a development branch and wish to obtain a DOI for your specific version we ask that you `contact us <mailto:anyone@underworldcode.org>`_ to make an interim release under the master DOI.

|DOI|_

.. |DOI| image:: https://zenodo.org/badge/DOI/10.5281/zenodo.1436039.svg
.. _DOI: https://doi.org/10.5281/zenodo.1436039

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, doi: 10.1007/s00024-002-8738-3.

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497.

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, doi: 10.1016/j.pepi.2007.06.009.


.. toctree::
    :hidden:
    :maxdepth: 1

    Geodynamics <Geodynamics>
    Numerical Methods <Numerics>
    Installation <Installation>
    User Guide <UserGuide>

    API Reference: Underworld <build/underworld>
    API Reference: Glucifer <build/glucifer>

    UW2 Github  <https://github.com/underworldcode/underworld2>
    UW2 Website <http://www.underworldcode.org>
    UW2 Zenodo <https://doi.org/10.5281/zenodo.1436039>


