# Application-specific makefile.  This file only needs to
# set the APPROOT, SRCS, HDRS, and EXEC variables properly
# (i.e. the location of the apps directory in relation to this Makefile), and
# then include the Makerules file from the main apps directory.
# All the real work in done in Makerules.  Things are setup
# this way because the build procedure for all apps is basically the same.


SRCS=makeprocs.c circular_buffer.c
EXEC=makeprocs.dlx.obj

$(warning "Including ../Makerules")
include ../Makerules

$(warning "Including $(APPROOT)/Makerules")
include $(APPROOT)/Makerules

$(warning "Done")
