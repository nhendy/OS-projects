default:
	cd test_fork; make

clean:
	cd test_fork; make clean

test_fork: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u test_fork.dlx.obj; ee469_fixterminal
