# Builds LeafSDTools.exe with the open CeGCC toolchain
# (https://github.com/palian-tech/cegcc-build, image ghcr.io/palian-tech/cegcc)
# instead of eMbedded Visual C++ 4:
#
#   make TARGET=arm    QY8XXX head units (Windows CE, ARM)  -> build/arm/
#   make TARGET=sh4    QY7XXX head units (Windows CE 5.0, SH-4) -> build/sh4/
#
# Mirrors the "Win32 (WCE ARMV4I) Release" and "Win32 (WCE SH4) Release"
# configurations of LeafSDTools/LeafSDTools.vcp.

TARGET ?= arm

SRCDIR := LeafSDTools
COMPAT := compat
BUILD  := build/$(TARGET)
OUT    := $(BUILD)/LeafSDTools.exe

ifeq ($(TARGET),arm)
CROSS ?= arm-mingw32ce-
# Display.cpp, WriteNAND.cpp and ProdInf.cpp are excluded from the ARMV4I
# configuration in the .vcp and belong to the SH4 build.
SRCS := \
  $(SRCDIR)/LeafSDTools.cpp \
  $(SRCDIR)/DisplayDirectDraw.cpp \
  $(SRCDIR)/ExitUpdateMode.cpp \
  $(SRCDIR)/Flash.cpp \
  $(SRCDIR)/Logger.cpp \
  $(SRCDIR)/ProdInf_QY8.cpp \
  $(SRCDIR)/ReadNAND.cpp \
  $(SRCDIR)/SDPinControl.cpp \
  $(SRCDIR)/StdAfx.cpp \
  $(SRCDIR)/Touch.cpp \
  $(SRCDIR)/UserSRAM.cpp \
  $(SRCDIR)/WriteNAND_QY8.cpp \
  $(COMPAT)/cxx_stubs.cpp
DEFS := -DQY8XXX -D_ARM_ -DARMV4I
PE_FORMAT := pei-arm-wince-little
DLLS := coredll|ddraw
DDRAW := $(BUILD)/libddraw.a
LDLIBS := $(DDRAW) -lcommctrl -lcoredll
else ifeq ($(TARGET),sh4)
CROSS ?= sh4-mingw32ce-
SRCS := \
  $(SRCDIR)/LeafSDTools.cpp \
  $(SRCDIR)/Display.cpp \
  $(SRCDIR)/ExitUpdateMode.cpp \
  $(SRCDIR)/Flash.cpp \
  $(SRCDIR)/Logger.cpp \
  $(SRCDIR)/ProdInf.cpp \
  $(SRCDIR)/ReadNAND.cpp \
  $(SRCDIR)/SDPinControl.cpp \
  $(SRCDIR)/StdAfx.cpp \
  $(SRCDIR)/Touch.cpp \
  $(SRCDIR)/UserSRAM.cpp \
  $(SRCDIR)/WriteNAND.cpp \
  $(COMPAT)/cxx_stubs.cpp
DEFS := -DSHx -DSH4 -D_SH4_
PE_FORMAT := pei-sh4-wince-little
DLLS := coredll|ceddk
DDRAW :=
LDLIBS := -lcommctrl -lceddk -lcoredll
else
$(error TARGET must be arm or sh4)
endif

CXX     := $(CROSS)g++
DLLTOOL := $(CROSS)dlltool
OBJDUMP := $(CROSS)objdump

OBJS := $(patsubst %.cpp,$(BUILD)/%.o,$(SRCS))

DEFS += -DUNICODE -D_UNICODE -D_WIN32_WCE=0x500 -DWINCE -DNDEBUG

# No exceptions or RTTI: the code uses neither, and this keeps libstdc++ out.
CXXFLAGS := -O2 -Wall -Wno-unused -fno-exceptions -fno-rtti \
            -include $(COMPAT)/cegcc.h $(DEFS) -I$(SRCDIR) -I$(COMPAT)

# The head unit has no CeGCC runtime DLLs, so libgcc is linked statically.
# Stripped and without a timestamp, so a given toolchain gives the same bytes.
LDFLAGS := -static-libgcc -s -Wl,--no-insert-timestamp

.PHONY: all clean check
all: $(OUT)

ifneq ($(DDRAW),)
$(DDRAW): $(COMPAT)/ddraw.def
	@mkdir -p $(dir $@)
	$(DLLTOOL) -d $< -l $@
endif

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS) $(DDRAW)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

# Fails if the EXE is not a Windows CE image for TARGET or imports a DLL that
# the head unit does not have.
check: $(OUT)
	@$(OBJDUMP) -f $(OUT) | grep -q '$(PE_FORMAT)' || \
	  { echo "not a Windows CE $(TARGET) PE image"; exit 1; }
	@extra=$$($(OBJDUMP) -p $(OUT) | sed -n 's/.*DLL Name: //p' | \
	  grep -v -i -x -E '($(DLLS))(\.dll)?'); \
	  if [ -n "$$extra" ]; then echo "unexpected imports: $$extra"; exit 1; fi
	@$(OBJDUMP) -p $(OUT) | sed -n 's/.*DLL Name: /import: /p'

clean:
	rm -rf build


BASE_IMAGE ?= build/LeafSDTools-base.img
PYTHON ?= python3
# One card image for both unit families: the ARM updater slot (QY8XXX) and
# the two SH-4 updater banks (QY7XXX).
CARD_IMAGE := build/LeafSDTools.img
ARM_EXE := build/arm/LeafSDTools.exe
SH4_EXE := build/sh4/LeafSDTools.exe

.PHONY: card-image
card-image:
	@test -s "$(BASE_IMAGE)" || { echo "missing BASE_IMAGE=$(BASE_IMAGE)"; exit 1; }
	$(MAKE) TARGET=arm check
	$(MAKE) TARGET=sh4 check
	$(PYTHON) ci/make-card-image.py "$(BASE_IMAGE)" "$(ARM_EXE)" \
	  "$(CARD_IMAGE)" --sh4-exe "$(SH4_EXE)"
	sha256sum "$(CARD_IMAGE)" > "$(CARD_IMAGE).sha256"
	gzip -n -9 -c "$(CARD_IMAGE)" > "$(CARD_IMAGE).gz"
	cat "$(CARD_IMAGE).sha256"
