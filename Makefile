ifndef OS # linux
EXECUTABLE_EXTENSIONS=
else ifeq ($(OS), Windows_NT) # windows
EXECUTABLE_EXTENSION=.exe
else
$(error OS not supported)
endif

ifndef OS # linux
RM=rm -f $(1)
else # windows
RM=if exist $(1) del $(1)
endif

# NOTE: currently default libjpeg-turbo install folder used but I left a..
#       .. feature request to add an environment variable
#       ^
#       https://libjpeg-turbo.org/Documentation/OfficialBinaries
#       https://github.com/libjpeg-turbo/libjpeg-turbo/issues/686
I:=../arguments-mini/ ../lcg-mini/ C:/libjpeg-turbo-gcc[64]/include/
L:=../arguments-mini/ ../lcg-mini/ C:/libjpeg-turbo-gcc[64]/lib/
l:=arguments-mini lcg-mini turbojpeg

# TODO: ifndef OS # linux
#       lcg.AppImage: lcg
#         #...
#       endif
lcg$(EXECUTABLE_EXTENSION): main.o
	gcc -o $@ $^ $(addprefix -L,$(L)) $(addprefix -l,$(l))

main.o: main.c
	gcc -c $< $(addprefix -I,$(I))

clean:
	$(call RM,main.o)
	$(call RM,lcg)
	$(call RM,lcg.exe)