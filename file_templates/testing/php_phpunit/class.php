{% load kdev_filters %}
<?php

{% block license_header %}
{% if license %}
/*

{{ license|lines_prepend:" * " }}
 */
{% endif %}
{% endblock license_header %}


class {{ name }} extends PHPUnit_Framework_TestCase
{
    {% for case in testCases %}

    public function test{{ case|capfirst }}()
    {


    }

    {% endfor %}
}


?>