default:
	cd ostests; make

clean:
	cd ostests; make clean

run_dbg: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -D Fs -u ostests.dlx.obj; ee469_fixterminal
run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u ostests.dlx.obj; ee469_fixterminal
