{% load kdev_filters %}
<?php


/*

{{ license|lines_prepend:" * " }}
 */


class {{ name }} extends PHPUnit_Framework_TestCase
{
    {% for case in testCases %}

    public function test{{ case|capfirst }}()
    {


    }

    {% endfor %}
}


?>