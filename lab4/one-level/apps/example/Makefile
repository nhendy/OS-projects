default:
	cd makeprocs; make
	cd hello_world; make
	cd test_two; make
	cd test_three; make
	cd test_four; make
	cd test_six; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd test_two; make clean
	cd test_three; make clean
	cd test_four; make clean
	cd test_six; make clean

run: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 5; ee469_fixterminal

test_two: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -D m -u test_two.dlx.obj; ee469_fixterminal

test_three: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u test_three.dlx.obj; ee469_fixterminal

test_four: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u test_four.dlx.obj; ee469_fixterminal

test_five: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 100; ee469_fixterminal

test_six: default
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 30; ee469_fixterminal
