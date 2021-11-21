# See file "doc/Makefile.doc" for longer and uncut description.
# Parameters:
# IFACE:	Programmer name, either "avrisp" or "avrdude".
# NAME:		Program target name.
# MCU:		micrcontroller, atmega88, etc.
# SPI:		SPI clock frequency, Hz.
# MICRO:	Micro source directory (optional).
# SRC:		Program sources.
# MSRC:		Micro source files (optional).
# HFUSE:	High fuse byte.
# LFUSE:	Low fuse byte.
# EFUSE:	Extended fuse byte.
# LOCK:		Lock bits (only AVRISPmkII).
# CFLAGS:	Compiler flags (optional).
# LDFLAGS:	Linker flags (optional).

# set defaults.
NAME	:= $(if $(NAME),$(NAME),firmware)
ISP	:= $(if $(ISP),$(ISP),200000)
IFACE	:= $(if $(IFACE),$(IFACE),avrisp)

OBJ	:=	\
		$(filter %.o, $(SRC:.c=.o))	\
		$(filter %.o, $(SRC:.cxx=.o))  	\
		$(filter %.o, $(MSRC:.c=.o))	\
		$(filter %.o, $(MSRC:.cxx=.o))  

#MICRO is optional.
CFLAGS_	:= $(CFLAGS) -O2 -mmcu=$(MCU) $(if $(MICRO),-I $(MICRO)) -I . -Wall
AVRDUDE	:= avrdude -p $(MCU:atmega=m) -c pony-stk200 -P lpt1
AVRISP	:= STK500 -cUSB -d$(MCU:at=AT) -I$(ISP)

all:	$(NAME).elf
#all:
#	@echo $(SRC)
#	@echo $(OBJ)
#	@echo $(OBJ_FILES)

#Sometimes MSRC is empty; the rules might interfere with existing files.
ifneq ($(strip $(MSRC)),)
%.o:	$(MICRO)/Micro/%.cxx
	avr-g++ $(CFLAGS_) -o $@ -c $<

%.o:	$(MICRO)/Micro/%.c
	avr-gcc $(CFLAGS_) -o $@ -c $<
endif

%.o:	%.cxx
	avr-g++ $(CFLAGS_) -o $@ -c $<

%.o:	%.c
	avr-gcc $(CFLAGS_) -o $@ -c $<

$(NAME).elf:	$(OBJ)
	avr-g++ $(CFLAGS_) $(LDFLAGS) -o $@ $^

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

# Program MCU
flash:	$(NAME).hex
ifeq ($(IFACE),avrisp)
	# note: AVRISPmkII.chm lists incorrect order of fuses.
	# here it is correct.
	$(AVRISP) -if$< -e -pf -vf $(if $(LFUSE),-f$(subst 0x,,$(HFUSE))$(subst 0x,,$(LFUSE)),) $(subst 0x,-E,$(EFUSE)) $(if $(LOCK),-l$(subst 0x,,$(LOCK)))
else
	$(AVRDUDE) -U flash:w:"$<":i $(if $(LFUSE),-U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m,) $(if $(EFUSE),-U efuse:w:$(EFUSE):m,)
endif

# Reset MCU
reset:
ifeq ($(IFACE),avrisp)
	$(AVRISP) -q
else
	$(AVRDUDE) -n
endif

clean:
	rm -f $(OBJ)
	rm -f $(NAME).elf
	rm -f $(NAME).hex
	rm -f *.*~
	rm -f *~
