# Application-specific makefile.  This file only needs to
# set the APPROOT, SRCS, HDRS, and EXEC variables properly
# (i.e. the location of the apps directory in relation to this Makefile), and
# then include the Makerules file from the main apps directory.
# All the real work in done in Makerules.  Things are setup
# this way because the build procedure for all apps is basically the same.


SRCS=consumer.c ../common/circular_buffer.c
EXEC=consumer.dlx.obj
INTERNAL_LIBS_OBJS=work/circular_buffer.o work/utils.o
# lab2/apps/example/Makerules
include ../Makerules

# lab2/apps/Makerules
include $(APPROOT)/Makerules
