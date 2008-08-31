''' Trivial example class intended to showcase the KDevelop4 PyUnit xTest 
    runner [TODO] '''

class NegativeAmountException(Exception):
	def __str__(self):
		return "Negative amount does not make sense."

class Money:
	def __init__(self, amount, currency):
		if amount > 0:
			self.amount_ = amount
			self.currency_ = currency
		else:
			raise NegativeAmountException

	def amount(self):
		return self.amount_

	def currency(self):
		return self.currency_

