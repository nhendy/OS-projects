
print-%:
	$(warning $(TARGET_DIRS))
	@echo $* = $($*)

# TODO:(nhendy) better way to loop over dirs
default:
	cd makeprocs; make
	cd injector; make
	@echo "Done"

clean:
	cd makeprocs; make clean
	cd injector; make clean
	@echo "Done"


run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 2 2; ee469_fixterminal
