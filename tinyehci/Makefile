clean:
	find . -name \*~ -exec rm -f {} \;
	find . -type l -exec rm -f {} \;
	make -C linux clean
	make -C base clean
	rm linux/modules.order
