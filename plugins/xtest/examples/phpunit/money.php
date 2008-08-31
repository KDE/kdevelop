<?php
class Money {
    var $amount;
    var $currency;

    function Money($amount, $currency) {
	if ($amount <= 0) {
	    throw new Exception("Negative amount does not make sense.");
	}
	$this->amount = $amount;
	$this->currency = $currency;
    }

    function amount() {
	return $this->amount;
    }

    function currency() {
	return $this->currency;
    }
}
?>
