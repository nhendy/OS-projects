default:
	cd test_failures; make

clean:
	cd test_failures; make clean

run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u test.dlx.obj; ee469_fixterminal
