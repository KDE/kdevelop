<?php
require_once 'PHPUnit/Framework.php';
require_once 'money.php';
 
class MoneyTest extends PHPUnit_Framework_TestCase {

    public function testCreate() {
	$m = new Money(5, "USD");
        $this->assertEquals(5, $m->amount());
	$this->assertEquals("USD", $m->currency());
    }
 
    /**
     * @expectedException Exception
     */
    public function testNegative() {
	$m = new Money(-5, "USD");
    }

    public function testZero() {
	$m = new Money(0, "USD");
	$this->assertEquals(0, $m->amount());
	$this->assertEquals("USD", $m->currency());
    }
}
?>
