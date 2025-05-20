## Makefile.am -- Process this file with automake to get Makefile.in

srcdir = @srcdir@
VPATH = @srcdir@

lib_LIBRARIES = libepos.a
libepos_a_SOURCES =  \
epos-af-lhcr.f   epos-bas-lhcr.f  epos-con-lhcr.f  \
epos-dky-lhcr.f  epos-dro-lhcr.f epos-ems-lhcr.f  \
epos-fra-lhcr.f  epos-hg-lhcr.f  epos-hnb-lhcr.f epos-ico-lhcr.f \
epos-ids-lhcr.f  epos-int-lhcr.f epos-jps-lhcr.f  \
epos-omg-lhcr.f  epos-par-lhcr.f epos-sem-lhcr.f  \
epos-rsh-lhcr.f  epos-qsh-lhcr.f epos-tim-lhcr.f  \
epos-uti-lhcr.f  epos-xan-lhcr.f epos-xpr-lhcr.f  \
eposm.f epos_j.f al.cpp memory.c

libepos_a_FFLAGS = @EPOS_FFLAGS@

if USE_HACAS
libepos_a_SOURCES += urqmdepos.f \
urqmd34/addpart.f   urqmd34/coload.f  urqmd34/detbal.f   \
urqmd34/getmass.f  urqmd34/hepnam.f  urqmd34/iso.f     urqmd34/output.f   \
urqmd34/scatter.f    urqmd34/uaaa.f urqmd34/angdis.f    urqmd34/boxprg.f   \
urqmd34/dwidth.f     urqmd34/getspin.f  urqmd34/init.f    urqmd34/ityp2pdg.f \
urqmd34/numrec.f   urqmd34/paulibl.f   urqmd34/siglookup.f  urqmd34/upmerge.f  \
urqmd34/anndec.f    urqmd34/cascinit.f   urqmd34/dectim.f   urqmd34/error.f  \
urqmd34/hepchg.f   urqmd34/input.f   urqmd34/jdecay2.f    urqmd34/proppot.f  \
urqmd34/string.f     urqmd34/upyth.f  urqmd34/blockres.f   \
urqmd34/delpart.f   urqmd34/hepcmp.f   urqmd34/make22.f  \
urqmd34/saveinfo.f  urqmd34/tabinit.f    urqmd34/whichres.f
libepos_a_FFLAGS += -I$(srcdir)/urqmd34
else
libepos_a_SOURCES += eposu_no.f
endif
 
nodist_libepos_a_SOURCES = epos.incems

BUILT_SOURCES = epos.incems
CLEANFILES = epos.incems

if USE_HIGHMEM
epos.incems:
	@cp -f $(srcdir)/epos.incems.high $(top_builddir)/epos/epos.incems
else
epos.incems:
	@cp -f $(srcdir)/epos.incems.low $(top_builddir)/epos/epos.incems
endif

EXTRA_DIST = epos.inc epos.incems.* epos.incico epos.incpar epos.incsem \
	     epos.iniev epos.initl  ak.h  epos.incxan idt.dt idky5.* idresi.dt\
	     epos.param epos.inchy epos.inirj epos.inics  epos.inirj.nf epos.inics.nf\
	     epos.inchy  epos.incho urqmd34/comres.f  urqmd34/coms.f  urqmd34/options.f\
	     urqmd34/colltab.f urqmd34/inputs.f urqmd34/newpart.f urqmd34/boxinc.f\
	     urqmd34/freezeout.f urqmd34/outcom.f urqmd34/comwid.f urqmd34/comnorm.f\
	     urqmd34/comstr.f urqmd34/tables.dat urqmd34/*.dat

README:
	@if test -f epos.lhc-r_cors.tar.gz  ; then \
	tar zxf epos.lhc-r_cors.tar.gz ; rm -f epos.lhc-r_cors.tar.gz ;\
	fi

