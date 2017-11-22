---
layout: home
title: Introduction
description: Course notes
# permalink: index.html   # make this the default home page which will be served at [link](/)
---

_This is a collection of tools to set up a web server in a docker container that mixes static html content and dynamic "literate-programming" content via jupyter notebooks._

# Introduction

[Jupyter](www.jupyter.org) notebooks blend well-formatted documentation with executable programming content. Notebooks not only provide a development environment for exploring new algorithms and implementation, they are an excellent way to introduce new software concepts through live demonstrations and example code.

[Docker](www.docker.com) is a mechanism for deploying complex software stacks as lightweight "apps" and managing  software dependencies in a consistent, reproducible manner. Docker guarantees the user experience will be identical to that shipped by the developer and eliminates installation inconsistencies.

These characteristics of Docker allow us to create complex software bundles and focus on the explanation of how they work, demonstrate the in-built algorithms, and provide tutorials that can be reused on new problems. We think this opens many doors for teaching basic computational methods as well as documenting technical software.

Under Windows and Macintosh, you can use the [Kitematic gui](https://kitematic.com) to locate, download and run docker images. I'm assuming this is how you are viewing this content as the gui / plug-and-play aspect of docker and kitematic is the whole point !

# What have we here ?

This is a demonstration of how to bundle python software into a Docker image and provide a simple user interface in the form of a static website and live jupyter notebooks.

The web pages are built from a simple script that converts a small collection of markdown files (which is also used in the notebooks to format content) into web pages via templates. These templates make it easier to modify the content without damaging the pieces that interact with the `jupyter notebook` server.

It should be possible to navigate freely between the web content and the live notebooks. In case you get lost in the notebooks, you can return to [this page](/) from the Jupyter logo.

The links in the Navigation bar will take you to  

   * [Home](/files/landing_pages/_site/Content/index.html) - this page
   * [Index](/fileslanding_pages/_site/Content/Map.html) - a list of pages / facilities
   * [Notebooks](/notebooks/Content/Notebooks/StartHere.ipynb) - a sample notebook (can be linked to other notebooks)
   * [Jupyter](/Content/Notebooks ) - the file browser and editor
   * [Terminal](/terminals/1) - a terminal running on the backend machine



# What is this for ?

You can [fork this project on github](https://github.com/lmoresi/docker-website-notebooks#fork-destination-box) and use it to build your own docker image. I think you can use this template to make live manuals and courses for interactive content - especially when you need to control the installation and ensure a uniform environment for your users. In the classroom, perhaps, where you only need to install docker itself and then you can be sure your students will all see the same thing !

Another use for this project is as a mechanism for distributing code for review, snapshot a version of your code to distribute as part of a publication, or for the purposes of benchmarking.

# Getting help

I can't promise much help, but you can [contact me by email](mailto:louis.moresi@unimelb.edu.au)
