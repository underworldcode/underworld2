## Underworld documentation pages




### Landing pages

The landing pages are intended for use in the docker environment to give the introductory user a simple entry-point to a running container. The site pages are
configured to run only through a jupyter notebook launched from the docs directory. 



`bundle exec jekyll build && bundle exec compass compile && bundle exec jekyll build`

jupyter notebook --NotebookApp.token='' --NotebookApp.allow_origin='*' --NotebookApp.default_url='/files/landing_pages/_site/Content/index.html'
