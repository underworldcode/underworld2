docs
====

Various user & developer domain documents and models. All models should run
successfully against corresponding version of Underworld.


test
----
These are more developer domain scripts, but advanced users might also find them useful.
For example, more thorough analytic tests might be housed here.


examples
--------
Examples generally demonstrate complete underworld model construction and usage.

Naming convention for example files:

```
a_b_textual_name.ipynb (.py)
```

where

a: Loose indication of difficulty level (1=novice, 3=advanced)
b: Unique & persistent integer identifier for example

Example: `2_04_Analytic Solutions.ipynb`


user_guide
----------
The user guide takes a more focussed look at various aspects of underworld
model construction. Jupyter notebooks are utilised to allow users to directly
interact with and modify content (temporarily). Where possible, visualisation
should be constructed using glucifer.


cheatsheet
----------
One pager outlining the most common Underworld usage patterns 
and classes.


development
-----------
Various documents usually more useful for Underworld developers, though
potentially of wider use. In particular docker related files are housed 
here. 


api_doc
-------
Sphinx generated API documentation is placed here. Note that these documentations
are not stored within the repository, and are instead generated when required. 
See document within folder for further information. 


install_guides
--------------
Installation instructions for various HPC facilities.
