{% load kdev_filters %}
/*

{{ license|lines_prepend:" * " }}
 */


{% with namespaces|join:"_"|default:"___"|add:"_"|cut:"____"|upper as uc_prefix %}
{% with namespaces|join:"_"|default:"___"|add:"_"|cut:"____"|lower as lc_prefix %}
{% with namespaces|join:"" as prefix %}
{% with prefix|add:name as full_name %}

/* inclusion guard */
#ifndef __{{ uc_prefix }}{{ name|upper }}_H__
#define __{{ uc_prefix }}{{ name|upper }}_H__


#include <glib-object.h>
/*
 * Potentially, include other headers on which this header depends.
 */


/*
 * Type macros.
 */
#define {{ uc_prefix }}TYPE_{{ name|upper }}                  ({{ lc_prefix }}{{ name|lower }}_get_type ())
#define {{ uc_prefix }}{{ name|upper }}(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), {{ uc_prefix }}TYPE_{{ name|upper }}, {{ full_name }}))
#define {{ uc_prefix }}IS_{{ name|upper }}(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), {{ uc_prefix }}TYPE_{{ name|upper }}))
#define {{ uc_prefix }}{{ name|upper }}_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), {{ uc_prefix }}TYPE_{{ name|upper }}, {{ full_name }}Class))
#define {{ uc_prefix }}IS_{{ name|upper }}_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), {{ uc_prefix }}TYPE_{{ name|upper }}))
#define {{ uc_prefix }}{{ name|upper }}_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), {{ uc_prefix }}TYPE_{{ name|upper }}, {{ full_name }}Class))


typedef struct _{{ full_name }}       {{ full_name }};
typedef struct _{{ full_name }}Class   {{ full_name }}Class;


struct _{{ full_name }}
{
  GObject parent_instance;


  /* instance members */
  {% for m in members %}
  {{ m.type }} {{ m.name }};
  {% endfor %}
};


struct _{{ full_name }}Class
{
  GObjectClass parent_class;

  /* class members */
  {% for f in functions %}
  {% if f.isVirtual %}
    {% with f.arguments as arguments %}
    {{ f.returnType|default:"void" }} (*{{ f.name }}) ({{ full_name }}* self{% if arguments %}, {% include "arguments_types_names.txt" %}{% endif %});
    {% endwith %}
  {% endif %}
  {% endfor %}

};


/* used by {{ uc_prefix }}TYPE_{{ name|upper }} */
GType {{ lc_prefix }}{{ name|lower }}_get_type (void);


/*
 * Method definitions.
 */
{% for f in functions %}
{% with f.arguments as arguments %}
{{ f.returnType|default:"void" }} {{ lc_prefix }}{{ name|lower }}_{{ f.name }}({{ full_name }}* self{% if arguments %}, {% include "arguments_types_names.txt" %}{% endif %});
{% endwith %}
{% endfor %}


#endif /* __{{ uc_prefix }}{{ name|upper }}_H__ */

{% endwith %}
{% endwith %}
{% endwith %}
{% endwith %}