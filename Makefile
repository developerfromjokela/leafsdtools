# Builds LeafSDTools.exe for QY8XXX head units (Windows CE, ARMV4I) with
# cegcc / mingw32ce instead of eMbedded Visual C++ 4.
#
#   make CROSS=arm-mingw32ce-
#
# Mirrors the "Win32 (WCE ARMV4I) Release" configuration of
# LeafSDTools/LeafSDTools.vcp. The SH4 (QY7) configuration is not covered:
# cegcc targets ARM only.

CROSS   ?= arm-mingw32ce-
CXX     := $(CROSS)g++
DLLTOOL := $(CROSS)dlltool
OBJDUMP := $(CROSS)objdump

SRCDIR := LeafSDTools
COMPAT := compat
BUILD  := build
OUT    := $(BUILD)/LeafSDTools.exe

# Sources of the ARMV4I configuration. Display.cpp, WriteNAND.cpp and
# ProdInf.cpp are excluded from it in the .vcp and belong to the SH4 build.
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

OBJS := $(patsubst %.cpp,$(BUILD)/%.o,$(SRCS))

DEFS := -DQY8XXX -DUNICODE -D_UNICODE -D_WIN32_WCE=0x500 \
        -DWINCE -D_ARM_ -DARMV4I -DNDEBUG

# No exceptions or RTTI: the code uses neither, and this keeps libstdc++ out.
CXXFLAGS := -O2 -Wall -Wno-unused -fno-exceptions -fno-rtti \
            -include $(COMPAT)/cegcc.h $(DEFS) -I$(SRCDIR) -I$(COMPAT)

# The head unit has no cegcc runtime DLLs, so libgcc is linked statically.
# The resulting EXE must import only COREDLL.dll and ddraw.dll.
LDFLAGS := -static-libgcc
DDRAW   := $(BUILD)/libddraw.a
LDLIBS  := $(DDRAW) -lcommctrl -lcoredll

.PHONY: all clean check
all: $(OUT)

$(DDRAW): $(COMPAT)/ddraw.def
	@mkdir -p $(dir $@)
	$(DLLTOOL) -d $< -l $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS) $(DDRAW)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

# Fails if the EXE is not a Windows CE ARM image or imports a DLL that the
# head unit does not have.
check: $(OUT)
	@$(OBJDUMP) -f $(OUT) | grep -q 'pei-arm-wince-little' || \
	  { echo "not a Windows CE ARM PE image"; exit 1; }
	@extra=$$($(OBJDUMP) -p $(OUT) | sed -n 's/.*DLL Name: //p' | \
	  grep -v -i -x -E '(coredll|ddraw)(\.dll)?'); \
	  if [ -n "$$extra" ]; then echo "unexpected imports: $$extra"; exit 1; fi
	@$(OBJDUMP) -p $(OUT) | sed -n 's/.*DLL Name: /import: /p'

clean:
	rm -rf $(BUILD)


BASE_IMAGE ?= build/LeafSDTools-base.img
PYTHON ?= python3
CARD_IMAGE := build/LeafSDTools-QY8XXX.img

.PHONY: card-image
card-image: check
	@test -s "$(BASE_IMAGE)" || { echo "missing BASE_IMAGE=$(BASE_IMAGE)"; exit 1; }
	@mkdir -p $(BUILD)
	$(PYTHON) ci/make-card-image.py "$(BASE_IMAGE)" "$(OUT)" "$(CARD_IMAGE)"
	sha256sum "$(CARD_IMAGE)" > "$(CARD_IMAGE).sha256"
	gzip -n -9 -c "$(CARD_IMAGE)" > "$(CARD_IMAGE).gz"
	cat "$(CARD_IMAGE).sha256"
