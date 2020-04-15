default:
	cd test_heap; make

clean:
	cd test_heap; make clean

test_heap: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u test_heap.dlx.obj; ee469_fixterminal
