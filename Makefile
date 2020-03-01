OS_DIR := $(shell pwd)
PROJ_NAME := $(shell grep -oP "(?<=TARGET = )\w+" testbench/wait_test/Makefile)

test_list = wait_test 

#test all 
test: 
	for test in $(test_list); do \
		$(MAKE) test -C testbench/$$test  ; \
	done

#just compile
compile:
	for test in $(test_list); do \
		$(MAKE) -C testbench/$$test  ; \
	done

#just compile
clean:
	for test in $(test_list); do \
		$(MAKE) clean -C testbench/$$test  ; \
	done