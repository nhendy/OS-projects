TARGET_DIRS:=$(abspath $(filter-out $(wildcard include*),$(filter-out $(wildcard Make*),$(shell ls)))

print-%:
	$(warning $(TARGET_DIRS))
	@echo $* = $($*)

default:
	for dir in $(TARGET_DIRS); do \
    	cd $$dir; make; \
	done

clean:
	for dir in $(TARGET_DIRS); do \
    	cd $$dir; make clean; \
	done

run:
	$(error Not implemented yet)
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 3; ee469_fixterminal
