import os

def declare_variables(variables, macro):
    """
    This is the hook for the functions

    - variables: the dictionary that contains the variables
    - macro: a decorator function, to declare a macro.
    """

    variables['baz'] = "John Doe"

# Note - will need to switch the /master in the docs repo
# to match the version when we tag / branch to match the uw versions

    uw_version = '2.6.0b'
    uw_gh_code = 'https://github.com/underworldcode/underworld2/tree/'+uw_version
    uw_gh_docs = 'https://github.com/underworldcode/underworld2-documentation/tree/master'
    uw_nbviewer_base_url = 'https://nbviewer.jupyter.org/github/underworldcode/underworld2-documentation/blob/master/content/docs/notebooks'
    uw_mybinder_base_url = 'https://mybinder.org/v2/gh/underworldcode/underworld2-documentation/master'

    variables['uw_version'] = uw_version


    @macro
    def embed_url_changeable(name, nb_height="600px", fname="nb_frame"):

## The substitution is messy because of the { } in the Javascript. this
## code creates a script to replace the embedded URL in the frame using the
## name of the frame itself.

        HTMLtemplate = """<div id="wrap" style="height:{nb_height}"><iframe src="{name}" id="{fname}" style="width:150%; transform: scale(0.666); transform-origin: top left; height:150%; border=0px; frameBorder=0px;"></iframe></div>
<script language="JavaScript" type="text/javascript">
<!--
  function changeFrame_{fname}(newPage) ??<
  document.getElementById("{fname}").src = newPage;
  ??>
//  -->
</script>
"""
        HTML = HTMLtemplate.format(nb_height=nb_height, name=name, fname=fname)
        HTML = HTML.replace("??<","{")
        HTML = HTML.replace("??>","}")

        return HTML

    @macro
    def embed_url(name, nb_height="600px", fname="nb_frame"):

## The substitution is messy because of the { } in the Javascript. this
## code creates a script to replace the embedded URL in the frame using the
## name of the frame itself.

        HTMLtemplate = """<div id="wrap" style="height:{nb_height}"><iframe src="{name}" id="{fname}" style="width:150%; transform: scale(0.666); transform-origin: top left; height:150%; border=0px; frameBorder=0px;"></iframe></div>"""
        HTML = HTMLtemplate.format(nb_height=nb_height, name=name, fname=fname)
        return HTML



    @macro
    def replace_embedded_url(name, fname, linktext):

        HTMLtemplate ="""<a href="{name}" onClick="changeFrame_{fname}(this.href); return false;">{linktext}</a>"""
        HTML = HTMLtemplate.format(name=name, fname=fname, linktext=linktext)

        return HTML


    @macro
    def nb_nbviewer_location(path2nb):
        path2nb = path2nb.replace(".html",".ipynb")
        if not path2nb.endswith(".ipynb"):
            path2nb = path2nb+".ipynb"

        return uw_nbviewer_base_url+"/"+path2nb


    @macro
    def nb_mybinder_location(path2nb):
        path2nb = path2nb.replace(".html",".ipynb")
        if not path2nb.endswith(".ipynb"):
            path2nb = path2nb+".ipynb"

        return uw_mybinder_base_url+"?filepath=/content/docs/"+path2nb

    @macro
    def nb_mybinder_badge(path2nb):
        url = nb_mybinder_location(path2nb)
        HTMLtemplate="""<a href="{url}"> <img src='https://mybinder.org/badge.svg'> </a>"""
        HTML = HTMLtemplate.format(url=url)
        return HTML


    @macro
    def figure(imgname, fstyle="", istyle="", caption=""):

        HTMLtemplate = """<div id="figure">
        <figure style="padding:10px; {}">
        <img src="{}" style="width:100%; display: block; margin-left: auto; margin-right: auto; {}"  >
        <figcaption style="font-size:smaller; font-style:oblique; width:100%;">
        {}
        </figcaption>
        </figure>
        </div>
        """


        HTML = HTMLtemplate.format( fstyle, imgname, istyle, caption)
        return HTML
