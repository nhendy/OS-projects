print-%:
	$(warning $(TARGET_DIRS))
	@echo $* = $($*)

default:
	cd consumer; make
	cd producer; make
	cd makeprocs; make
	@echo "Done"

clean:
	cd consumer; make clean
	cd producer; make clean
	cd makeprocs; make clean
	@echo "Done"


run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 2; ee469_fixterminal
