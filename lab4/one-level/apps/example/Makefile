default:
	cd makeprocs; make
	cd hello_world; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean

run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -D m -u makeprocs.dlx.obj 1; ee469_fixterminal
