OS_DIR := $(shell pwd)
PROJ_NAME := $(shell grep -oP "(?<=TARGET = )\w+" testbench/Makefile)


all:
	$(MAKE) -C testbench 
	@echo $(PROJ_NAME)
	sudo ${ST_DIR}/st-flash --format ihex write testbench/build/$(PROJ_NAME).hex


