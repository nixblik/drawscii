.PHONY: all
all: build


ifeq ($(wildcard config.mk),)
$(warning warning - configure must be run before make)
clean:
distclean:
else


include config.mk

qbsflags += $(QBSFLAGS) config:make
qbsargs  += $(QBSARGS)
qbsbg    := $(builddir)/make/make.bg



$(qbsbg): $(qbscf)
	$(QBS) resolve $(qbsflags) profile:make $(qbsargs)

%/compile_commands.json: $(qbsbg) force
	$(QBS) generate -g clangdb $(qbsflags)
	mv "$(builddir)/make/compile_commands.json" "$@"

.PHONY: force
force:

.PHONY: build
build: $(qbsbg)
	$(QBS) build --no-install $(qbsflags)

.PHONY: html
html: $(qbsbg)
	$(QBS) build --no-install $(qbsflags) -p doc

.PHONY: check
check: $(qbsbg)
	$(QBS) build -j 1 --no-install $(qbsflags) -p autotest-runner

ifeq ($(MAKECMDGOALS),run)
ifeq ($(strip $(PRODUCT)),)
$(error error - missing argument PRODUCT to run)
endif
endif

.PHONY: run
run: $(qbsbg)
	$(QBS) run --no-build $(qbsflags) -p $(PRODUCT) -- $(ARGS)

.PHONY: install
install: $(qbsbg)
	$(QBS) install --no-build $(qbsflags) --install-root "$(DESTDIR)"

.PHONY: clean
clean:
	-$(QBS) clean $(qbsflags)

.PHONY: distclean
distclean: clean
	-rm -rf "$(builddir)/make"
	-rmdir "$(builddir)"


endif
