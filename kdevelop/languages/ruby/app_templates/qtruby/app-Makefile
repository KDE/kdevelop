uis := $(wildcard *.ui)

all: $(uis)
	@for name in $^; do rbuic $$name > `echo $$name | awk -F '\.ui' '{ print $$1 }' 2> /dev/null`.rb; echo "Running rbuic on $$name"; done

clean: $(uis)
	@for name in $^; do rm -f `echo $$name | awk -F '\.ui' '{ print $$1 }' 2> /dev/null`.rb; done
