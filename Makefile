#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment.")
endif

#---------------------------------------------------------------------------------
PREFIX		:=	aarch64-none-elf-
export PATH	:=	$(DEVKITPRO)/devkitA64/bin:$(DEVKITPRO)/tools/bin:$(PATH)

export CC	:=	$(PREFIX)gcc
export CXX	:=	$(PREFIX)g++
export AS	:=	$(PREFIX)as
export AR	:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
export STRIP	:=	$(PREFIX)strip

ARCH		:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS		:=	-g -Wall -O2 -ffunction-sections $(ARCH) -D__SWITCH__
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
LDFLAGS		:=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := -lnx
LIBDIRS		:=	$(DEVKITPRO)/libnx

#---------------------------------------------------------------------------------
TARGET      := PKHeX-NX
BUILD       := build
SOURCES     := source
INCLUDES    := include

APP_TITLE   := PKHeX-NX
APP_AUTHOR  := You
APP_VERSION := 0.1.0
#---------------------------------------------------------------------------------

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD := $(CXX)

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@echo "Using compiler: $(CXX)"
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf

else

DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
	@echo creating ... $(notdir $@)
	@elf2nro $(OUTPUT).elf $@ --nacp=$(OUTPUT).nacp --icon=$(TOPDIR)/icon.jpg

$(OUTPUT).nacp:
	@echo creating ... $(notdir $@)
	@nacptool --create "$(APP_TITLE)" "$(APP_AUTHOR)" "$(APP_VERSION)" $@

$(OUTPUT).elf: $(OFILES)
	@echo linking $(notdir $@)
	@$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

main_dat.o: $(TOPDIR)/romfs/main.dat
	@echo embedding $(notdir $<)
	@cd $(TOPDIR)/romfs && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 main.dat $(CURDIR)/main_dat.o

mainHeader_dat.o: $(TOPDIR)/romfs/mainHeader.dat
	@echo embedding $(notdir $<)
	@cd $(TOPDIR)/romfs && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 mainHeader.dat $(CURDIR)/mainHeader_dat.o

landname_dat.o: $(TOPDIR)/romfs/landname.dat
	@echo embedding $(notdir $<)
	@cd $(TOPDIR)/romfs && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 landname.dat $(CURDIR)/landname_dat.o

%.o: %.cpp
	@echo compiling $(notdir $<)
	@$(CXX) -MMD -MP $(CXXFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.c
	@echo compiling $(notdir $<)
	@$(CC) -MMD -MP $(CFLAGS) $(INCLUDE) -c $< -o $@

%.o: %.s
	@echo assembling $(notdir $<)
	@$(AS) $(ASFLAGS) -c $< -o $@

-include $(DEPENDS)

endif
personal_dat.o: $(TOPDIR)/romfs/personal.dat
	@echo embedding $(notdir $<)
	@cd $(TOPDIR)/romfs && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 personal.dat $(CURDIR)/personal_dat.o

personalHeader_dat.o: $(TOPDIR)/romfs/personalHeader.dat
	@echo embedding $(notdir $<)
	@cd $(TOPDIR)/romfs && $(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 personalHeader.dat $(CURDIR)/personalHeader_dat.o
