OS_DIR := $(shell pwd)
PROJ_NAME := $(shell grep -oP "(?<=TARGET = )\w+" testbench/Makefile)


all:
	$(MAKE) -C testbench/wait_test 
	@echo $(PROJ_NAME)
	sudo ${ST_DIR}/st-flash --format ihex write testbench/wait_test/build/$(PROJ_NAME).hex


