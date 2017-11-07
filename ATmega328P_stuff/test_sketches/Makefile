sketch_name = test

symlink: ../include remlink
	ln -s -r ../include/* $(sketch_name)

remlink:
	find $(sketch_name) -type l -delete
