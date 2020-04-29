default:
	cd file_test; make

clean:
	cd file_test; make clean

run_dbg: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -D F -u file_test.dlx.obj; ee469_fixterminal
run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u file_test.dlx.obj; ee469_fixterminal
