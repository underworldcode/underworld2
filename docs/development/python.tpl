{%- extends 'null.tpl' -%}

{% block header %}
# coding: utf-8
{% endblock header %}

{% block input %}
{{ cell.source | ipython2python }}
{% endblock input %}

{% block markdowncell scoped %}
{{ cell.source | comment_lines }}
{% endblock markdowncell %}
