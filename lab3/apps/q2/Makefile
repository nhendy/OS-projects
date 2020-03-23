default:
	cd makeprocs; make
	cd injector_1; make
	cd injector_2; make
	cd reaction_1; make
	cd reaction_2; make
	cd reaction_3; make
	@echo "Done"
clean:
	cd makeprocs; make clean
	cd injector_1; make clean
	cd injector_2; make clean
	cd reaction_1; make clean
	cd reaction_2; make clean
	cd reaction_3; make clean
	@echo "Done"
run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a  -u makeprocs.dlx.obj 1 4; ee469_fixterminal
	# cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 1 4; ee469_fixterminal
