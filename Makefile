ifndef OS # linux
LIBRARY_EXTENSION=.a
EXECUTABLE_EXTENSIONS=
else ifeq ($(OS), Windows_NT) # windows
LIBRARY_EXTENSION=.lib
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
I:=C:/libjpeg-turbo-gcc[64]/include/
L:=C:/libjpeg-turbo-gcc[64]/lib/
l:=turbojpeg

LIBRARIES:=../arguments-mini/libarguments-mini$(LIBRARY_EXTENSION) ../lcg-mini/liblcg-mini$(LIBRARY_EXTENSION)

#******************************************************************************

I+= $(dir $(LIBRARIES))
L+= $(dir $(LIBRARIES))
l+= $(patsubst lib%$(LIBRARY_EXTENSION),%,$(notdir $(LIBRARIES)))

# TODO: ifndef OS # linux
#       lcg.AppImage: lcg
#         #...
#       endif
lcg$(EXECUTABLE_EXTENSION): main.o
	gcc -o $@ $^ $(addprefix -L,$(L)) $(addprefix -l,$(l))

main.o: main.c $(LIBRARIES)
	gcc -c $< $(addprefix -I,$(I))

clean:
	$(call RM,main.o)
	$(call RM,lcg)
	$(call RM,lcg.exe)