# Go parameters
GOBUILD=	$(GO) build
GOCLEAN=	$(GO) clean
GOTEST=		$(GO) test
GOGET=		$(GO) get
BINARY_NAME=	ops

all: build

build: deps
	$(GOBUILD)

test: build
	$(GOTEST) -v

post-clean:
	$(Q) $(GOCLEAN)
	$(Q) $(RM) -rf .staging
	$(Q) $(RM) -f $(BINARY_NAME) image ../runtime/soop.data

deps:
	$(GOGET) github.com/spf13/cobra
	$(GOGET) github.com/vishvananda/netlink
	$(GOGET) github.com/jstemmer/go-junit-report
	$(GOGET) github.com/d2g/dhcp4
	$(GOGET) github.com/d2g/dhcp4client
	$(GOGET) github.com/go-errors/errors
	$(GOGET) github.com/cheggaaa/pb
	$(GOGET) github.com/nanovms/ops/lepton

.PHONY: build test clean deps

include ../../rules.mk
