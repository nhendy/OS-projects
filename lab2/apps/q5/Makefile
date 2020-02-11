
print-%:
	$(warning $(TARGET_DIRS))
	@echo $* = $($*)

# TODO:(nhendy) better way to loop over dirs
default:
	cd makeprocs; make
	cd injector; make
	cd reactions; make
	@echo "Done"

clean:
	cd makeprocs; make clean
	cd injector; make clean
	cd reactions; make clean
	@echo "Done"


run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 3 3; ee469_fixterminal
