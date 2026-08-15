# libmuslim -- single-header C libraries, no build system required to *use* them.
#
# REQUIRES GNU MAKE (3.81 or newer -- what macOS ships).
# This file uses $(origin), $(addprefix) and order-only prerequisites, none of
# which are POSIX. BSD make (bmake, the default `make` on FreeBSD, NetBSD and
# OpenBSD) will not parse it; use `gmake` there. Rewriting for POSIX make would
# cost the pattern rules for little gain, so the dependency is documented rather
# than removed.
#
# This Makefile exists to enforce the claims the project makes, not to build a
# product. Every target below corresponds to something README.md or ROADMAP.md
# asserts and which was previously checked only by a human remembering to run a
# command. `prayertimes.h` shipped broken under its own documented C11 command
# because nobody ran it.
#
#   make            same as `make check`
#   make check      everything below, in order
#   make test       build all tests strict-C11 and run them
#   make cxx        compile every header as C++17
#   make examples   build the examples
#   make baseline   regenerate the research CSV and compare against the committed one
#   make clean      remove build artifacts
#
# Override the compiler to check portability:
#   make check CC=clang CXX=clang++

# make predefines CC=cc and CXX=g++ with origin "default", so `?=` never fires.
# Check the origin instead, which still honours a command-line or environment
# override such as `make check CC=clang CXX=clang++`.
ifeq ($(origin CC),default)
CC = gcc
endif
ifeq ($(origin CXX),default)
CXX = g++
endif
CSTD     ?= -std=c11
CXXSTD   ?= -std=c++17
WARN     ?= -Wall -Wextra -Wpedantic
OPT      ?= -O2
LDLIBS   ?= -lm
BUILD    ?= build

CFLAGS   = $(CSTD) $(WARN) $(OPT)
# -Wpedantic on C++ flags unrelated header noise in some toolchains; the C
# build already covers pedantic conformance, so the C++ pass checks that the
# headers *compile as C++* at all, which is what the README advertises.
CXXFLAGS = $(CXXSTD) -Wall -Wextra $(OPT)

TESTS    = test_hijri test_ephemeris_oracle test_prayertimes test_timezone
EXAMPLES = hijri_example prayertimes_example
HEADERS  = hijri.h prayertimes.h timezone.h

# Separate output directories so each pattern rule matches a distinct target
# pattern. With a single $(BUILD)/% both rules below matched, and which one won
# depended on GNU make picking the first rule whose prerequisite exists --
# correct, but it would have silently preferred tests/ if a basename ever
# collided with one in examples/.
TEST_BINS    = $(addprefix $(BUILD)/tests/,$(TESTS))
EXAMPLE_BINS = $(addprefix $(BUILD)/examples/,$(EXAMPLES))

.PHONY: all check test cxx examples baseline fielddocs clean

all: check

check: test cxx examples baseline fielddocs
	@echo "OK  all checks passed ($(CC) / $(CXX))"

$(BUILD)/tests $(BUILD)/examples:
	@mkdir -p $@

$(BUILD)/tests/%: tests/%.c $(HEADERS) | $(BUILD)/tests
	$(CC) $(CFLAGS) $< $(LDLIBS) -o $@

$(BUILD)/examples/%: examples/%.c $(HEADERS) | $(BUILD)/examples
	$(CC) $(CFLAGS) $< $(LDLIBS) -o $@

# Every test binary must build warning-free and exit zero.
test: $(TEST_BINS)
	@fail=0; for t in $(TEST_BINS); do \
	  if $$t > /dev/null 2>&1; then echo "  PASS  $$t"; \
	  else echo "  FAIL  $$t"; $$t 2>&1 | tail -20 | sed 's/^/        /'; fail=1; fi; \
	done; \
	if [ $$fail -ne 0 ]; then echo "FAIL  one or more tests failed"; exit 1; fi

# README and ROADMAP both advertise C++17 compatibility. Before this target
# existed, nothing in the repository ever compiled a header as C++.
cxx: $(HEADERS) | $(BUILD)/tests
	@printf '#define HIJRI_IMPLEMENTATION\n#include "hijri.h"\n' > $(BUILD)/tests/cxx_all.cpp
	@printf '#define PRAYERTIMES_IMPLEMENTATION\n#include "prayertimes.h"\n' >> $(BUILD)/tests/cxx_all.cpp
	@printf '#define MUSLIM_TIMEZONE_IMPLEMENTATION\n#include "timezone.h"\n' >> $(BUILD)/tests/cxx_all.cpp
	@printf 'int main(void){return 0;}\n' >> $(BUILD)/tests/cxx_all.cpp
	$(CXX) $(CXXFLAGS) -I. $(BUILD)/tests/cxx_all.cpp $(LDLIBS) -o $(BUILD)/tests/cxx_all
	@echo "  PASS  headers compile as C++17"

examples: $(EXAMPLE_BINS)
	@echo "  PASS  examples build"

# docs/research/hijri-2020-2025-baseline.csv is generated and committed so that
# changes to it show up in review. Any change to the ephemeris, the predicates,
# or the evening calculation moves it. This is the check that catches that.
#
# PLATFORM-DEPENDENT: cmp is verified byte-identical between glibc 2.44 and
# musl 1.2.6 on x86-64, the two libm implementations measured against each
# other. Other platforms and architectures are untested and may differ. See
# the DETERMINISM section in hijri.h for the full contract. CI runs this
# target on Linux only.
baseline: $(BUILD)/tests/hijri_research_probe
	@$(BUILD)/tests/hijri_research_probe > $(BUILD)/baseline.csv
	@if cmp -s $(BUILD)/baseline.csv docs/research/hijri-2020-2025-baseline.csv; then \
	  echo "  PASS  research baseline matches the committed CSV"; \
	else \
	  echo "FAIL  research baseline differs from docs/research/hijri-2020-2025-baseline.csv"; \
	  echo "      If the change was intended, regenerate it:"; \
	  echo "        make baseline-update"; \
	  diff docs/research/hijri-2020-2025-baseline.csv $(BUILD)/baseline.csv | head -10 | sed 's/^/      /'; \
	  exit 1; \
	fi

.PHONY: baseline-update
baseline-update: $(BUILD)/tests/hijri_research_probe
	$(BUILD)/tests/hijri_research_probe > docs/research/hijri-2020-2025-baseline.csv
	@echo "  baseline regenerated -- review the diff before committing"

# Every field of HijriSunPosition, HijriMoonPosition and HijriEveningParameters
# must carry a comment, either trailing on the field's own line or as a pure
# comment line directly above it. In a single-header library the header IS the
# API contract, so an undocumented field is a gap in that contract. A trailing
# comment on the PREVIOUS field must not count towards the field below it,
# which is the bug the first draft of this check had.
#
# Mutation record: with the comment on HijriEveningParameters.jd_sunset_ut
# deleted, `make fielddocs` produced (and the comment was restored after):
#   FAIL  HijriEveningParameters has 1 undocumented field(s)
.PHONY: fielddocs
fielddocs: hijri.h
	@awk '\
	  /^typedef struct \{/ { inb=1; prevcomment=0; bad=0; next } \
	  /^\} Hijri(SunPosition|MoonPosition|EveningParameters);/ { \
	      if (inb) { name=$$2; sub(";","",name); \
	        if (bad>0) printf "  FAIL  %s has %d undocumented field(s)\n", name, bad; \
	        total+=bad } \
	      inb=0; next } \
	  inb { \
	      ispure = ($$0 ~ /^[[:space:]]*(\/\*|\*)/); \
	      isfield = ($$0 ~ /;[[:space:]]*$$/ || $$0 ~ /;[[:space:]]*\/\*/) && !ispure; \
	      if (isfield && $$0 !~ /\/\*/ && prevcomment == 0) bad++; \
	      if ($$0 ~ /^[[:space:]]*$$/) next; \
	      prevcomment = ispure; next } \
	  END { if (total>0) exit 1; print "  PASS  every public struct field is documented" } \
	' hijri.h

clean:
	rm -rf $(BUILD)
