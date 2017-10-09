sketch_name = test

symlink: ../lib ../include remlink
	ln -s -r ../lib/* $(sketch_name)
	ln -s -r ../include/* $(sketch_name)

remlink:
	find $(sketch_name) -type l -delete
