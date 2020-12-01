#!/usr/bin/make -f

export QBS := qbs
QBSFLAGS := 
QBSARGS := 
export QT_SELECT := qt5
QMAKE := /usr/bin/qmake

builddir := /home/uwe/code/draawsci/tmp
prefix := /usr/local
exec_prefix := /usr/local
bindir := /usr/local/bin
sbindir := /usr/local/sbin
libexecdir := /usr/local/libexec
sysconfdir := /usr/local/etc
sharedstatedir := /usr/local/com
localstatedir := /usr/local/var
runstatedir := /usr/local/var/run
libdir := /usr/local/lib
includedir := /usr/local/include
datarootdir := /usr/local/share
datadir := /usr/local/share
pkgdatadir := /usr/local/share/draawsci
infodir := /usr/local/share/info
localedir := /usr/local/share/locale
mandir := /usr/local/share/man
docdir := /usr/local/share/doc/draawsci
htmldir := /usr/local/share/doc/draawsci
dvidir := /usr/local/share/doc/draawsci
pdfdir := /usr/local/share/doc/draawsci
psdir := /usr/local/share/doc/draawsci

qbscfdir := $(builddir)/.qbscf
qbscf := $(qbscfdir)/.dir
qbscfflags := --settings-dir "$(qbscfdir)"
qbsflags := $(qbscfflags) -d "$(builddir)"
qbsargs := qbs.installPrefix:""
qbsargs += project.bindir:"$(bindir)"

$(qbscf): /home/uwe/code/draawsci/config.mk
	$(QBS) setup-toolchains $(qbscfflags) --detect
	$(QBS) config $(qbscfflags) --unset profiles.make
	$(QBS) config $(qbscfflags) profiles.make.warningLevel all
	$(QBS) setup-qt $(qbscfflags) "$(QMAKE)" make
	$(QBS) config $(qbscfflags) --export $(qbscf)

.PHONY: distclean clean-qbscf
distclean: clean-qbscf
clean-qbscf:
	-rm -rf "$(qbscfdir)"
