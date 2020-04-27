default:
	cd ostests; make

clean:
	cd ostests; make clean

run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -D F -u ostests.dlx.obj; ee469_fixterminal
