{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#include "{{ output_file_header }}"


{% with namespaces|join:"_"|default:"___"|add:"_"|cut:"____"|upper as uc_prefix %}
{% with namespaces|join:"_"|default:"___"|add:"_"|cut:"____"|lower as lc_prefix %}
{% with namespaces|join:"" as prefix %}
{% with prefix|add:name as full_name %}


#define {{ uc_prefix }}{{ name|upper }}_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), {{ uc_prefix }}TYPE_{{ name|upper }}, {{ full_name }}Private))


struct _{{ full_name }}Private
{
    /* private members */
};




/* 
 * forward definitions
 */
G_DEFINE_TYPE ({{ full_name }}, {{ lc_prefix }}{{ name|lower }}, G_TYPE_OBJECT);


/*
/* forward declarations of default virtual methods 
 */


{% for f in functions %}
{% if f.isVirtual %}
  {% with f.arguments as arguments %}
  {{ f.returnType|default:"void" }} {{ lc_prefix }}{{ name|lower }}_real_{{ f.name }}({{ full_name }}* self{% if arguments %}, {% include "arguments_types_names.txt" %}{% endif %});
  {% endwith %}
{% endif %}
{% endfor %}


static void {{ lc_prefix }}{{ name|lower }}_class_init ({{ full_name }}Class *klass)
{
  g_type_class_add_private (klass, sizeof ({{ full_name }}Private));
}


static void
{{ lc_prefix }}{{ name|lower }}_dispose (GObject *gobject)
{
  {{ full_name }} *self = {{ uc_prefix }}{{ name|upper }} (gobject);


  /* 
   * In dispose, you are supposed to free all types referenced from this
   * object which might themselves hold a reference to self. Generally,
   * the most simple solution is to unref all members on which you own a 
   * reference.
   */


  /* Chain up to the parent class */
  G_OBJECT_CLASS ({{ lc_prefix }}{{ name|lower }}_parent_class)->dispose (gobject);
}


static void
{{ lc_prefix }}{{ name|lower }}_finalize (GObject *gobject)
{
  {{ full_name }} *self = {{ uc_prefix }}{{ name|upper }} (gobject);


  /* Chain up to the parent class */
  G_OBJECT_CLASS ({{ lc_prefix }}{{ name|lower }}_parent_class)->finalize (gobject);
}


static void
{{ lc_prefix }}{{ name|lower }}_init ({{ full_name }} *self)
{
  self->priv = {{ uc_prefix }}{{ name|upper }}_GET_PRIVATE (self);


  /* initialize all public and private members to reasonable default values. */


  /*
   * Default implementations for virtual methods 
   * For pure-virtual functions, set these to NULL
   */
  {% for f in functions %}
  {% if f.isVirtual %}
    klass->{{ f.name }} = {{ lc_prefix }}{{ name|lower }}_real_{{ f.name }};
  {% endif %}
  {% endfor %}
}

{% for f in functions %}
{% with f.arguments as arguments %}
{{ f.returnType|default:"void" }} {{ lc_prefix }}{{ name|lower }}_{{ f.name }}({{ full_name }}* self{% if arguments %}, {% include "arguments_types_names.txt" %}{% endif %})
{
    g_return_if_fail ({{ uc_prefix }}IS_{{ name|upper }} (self));

    {% if f.isVirtual %}

    {{ uc_prefix }}{{ name|upper }}_GET_CLASS (self)->{{ f.name }} (self{% if arguments %}, {% include "arguments_names.txt" %}{% endif %});
    {% endif %}
}

{% if f.isVirtual %}
{{ f.returnType|default:"void" }} {{ lc_prefix }}{{ name|lower }}_real_{{ f.name }}({{ full_name }}* self{% if arguments %}, {% include "arguments_types_names.txt" %}{% endif %})
{
    /* Default implementation for the virtual method {{ f.name }} */
}
{% endif %}
{% endwith %}
{% endfor %}

{% endwith %}
{% endwith %}
{% endwith %}
{% endwith %}