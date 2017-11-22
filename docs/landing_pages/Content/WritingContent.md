---
layout: home
title: How to Write Content
author: Louis Moresi
description: How to Write Content
---

_The content of the web pages that are used for documentation is built from markdown using `jekyll` - a choice we make because it is can be deployed automatically on github webpages. Markdown is also the format used to write content for notebooks. Mathematics is included as latex using simple `mathjax` tags but this does require a live internet connection_

# Overview

Standard `markdown` will be converted to `html` via `jekyll`. Additional formatting occurs through the use of `liquid` tags (for example to highlight code). These are parsed at the build stage. If you want to change the formatting of the website then it will be necessary to understand how this works in detail. Otherwise, the following examples should be enough !

# Some example content

The usual [markdown formatting styles](https://daringfireball.net/projects/markdown/basics) work: _emphasis_, **bold** etc.

Lists are simple

   * they can include [links](www.google.com)
   * inline `code`
   * or even mathematical symbols / equations: \\( e^{i\pi} + 1 = 0 \\)

Numbered lists

   1. with [the other sort of links][RefLink]
   1. or formatted with `html` tags <i>like this</i>

[Reflink]: www.google.com  

The source code for the above section is:

{% highlight markdown %}

The usual [markdown formatting styles](https://daringfireball.net/projects/markdown/basics)
work: _emphasis_, **bold** etc.

Lists are simple

   * they can include [links](www.google.com)
   * inline `code`
   * or even mathematical symbols / equations: \\( e^{i\pi} + 1 = 0 \\)

Numbered lists

   1. with [the other sort of links][RefLink]
   1. or formatted with `html` tags <i>like this</i>

[Reflink]: www.google.com  

{% endhighlight %}

# Mathematics

We can also include mathematics because the [mathjax](http://mathjax.readthedocs.org/en/latest/) scripts have already been included in the headers of all the web pages (and are also enabled by default in the notebook markdown cells).

Inline mathematics works by using `\\(` and `\\)` symbols (the escaped version of what would normally be used in \\( \LaTeX \\) ). Display equations use `\\[` and `\\]` but it is also possible to use `\begin{equation}` and `\end{equation}` (which also allows you to use equation numbers) and other environments.

\\[ A = \pi r ^ 2 \\]

\begin{equation}
    A = \pi r ^ 2  \label{eq:area-of-a-circle}
\end{equation}

The latter form also allows us to create references to any equation (e.g. equation \\( \ref{eq:area-of-a-circle} \\) ) in the current _page_ context.

The above paragraphs are rendered from:

{% highlight markdown %}

Inline mathematics works by using `\\(` and `\\)` symbols
(the escaped version of what would normally be used in \\( \LaTeX \\) ).
Display equations use `\\[` and `\\]` but it is also possible to use
`\begin{equation}` and `\end{equation}` (which also allows you to use equation numbers)
and other environments.

\\[ A = \pi r ^ 2 \\]

\begin{equation}
    A = \pi r ^ 2  \label{eq:area-of-a-circle}
\end{equation}

The latter form also allows us to create references to any equation
(e.g. equation \\( \ref{eq:area-of-a-circle} \\) )
in the current _page_ context.

{% endhighlight %}

Mathjax is pretty smart but it does not get to see the web pages until they have been rendered by `jekyll` and so sometimes things can go wrong. Equations which contain `_` and `*` need to use extra whitespace to avoid being interpreted as emphasis marks. Use `\nonumber` instead of using the `*` form of mathematical environments. Things work surprisingly well but not every time !





# Code highlighting

The `jekyll` system renders code blocks which have been suitably tagged with `liquid`
markup. For example:

{% highlight liquid %}
{ % highlight latex % }
% Navier Stokes equation
\frac{\partial {\bf u}}{\partial t}  +
\left( {\bf u}. \nabla  \right) {\bf u}   =
-\frac{1}{\rho } \nabla p  +  \textbf{F}  + \nu \nabla ^ 2 {\bf u}
{ % endhighlight % }
{% endhighlight %}

produces

{% highlight latex %}
% Navier Stokes equation
\frac{\partial {\bf u}}{\partial t}  +
\left( {\bf u}. \nabla  \right) {\bf u}   =
-\frac{1}{\rho } \nabla p  +  \textbf{F}  + \nu \nabla ^ 2 {\bf u}
{% endhighlight %}

aka

\\[
\frac{\partial {\bf u}}{\partial t}  +
\left( {\bf u}. \nabla  \right) {\bf u}   =
-\frac{1}{\rho } \nabla p  +  \textbf{F}  + \nu \nabla ^ 2 {\bf u}
\\]



The list of 'known' languages (and the tags to use to identify them) are found [here](https://github.com/jneen/rouge/wiki/List-of-supported-languages-and-lexers)

{% highlight python %}

# Import the underworld system
import underworld as uw

# Take a look at the documentation
help(uw)

# Write some code
print "Hello Underworld"

{% endhighlight %}


# Some further reading

### Links

[Markdown Website](http://daringfireball.net/projects/markdown/)  
[Mathjax Website](http://docs.mathjax.org)  
[Jupyter Notebooks](http://www.jupyter.org)  
