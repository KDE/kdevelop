{% load kdev_filters %}
{% include "license_header_cpp.txt" %}


#ifndef {{ private_class_name|upper}}_H
#define {{ private_class_name|upper}}_H


{% include "namespace_open_cpp.txt" %}


class {{ private_class_name }}
{
public:
    {{ private_class_name }}({{ name }}* q);
    virtual ~{{ private_class_name }}();


    {% for method in private_methods %}
        {% include "declaration.txt" %}
    {% endfor %}


    {% for member in members %}
    {{ member.type }} {{ member.name }};
    {% endfor %}


private:
    class {{ name }}* const q;
};


{% include "namespace_close_cpp.txt" %}


#endif // {{ private_class_name|upper}}_H
