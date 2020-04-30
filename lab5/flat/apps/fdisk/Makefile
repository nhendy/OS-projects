default:
	cd fdisk; make

clean:
	cd fdisk; make clean

run: default
	cd ../../os; make; cd ../bin; dlxsim -x os.dlx.obj -a -x -D F -u fdisk.dlx.obj; ee469_fixterminal
