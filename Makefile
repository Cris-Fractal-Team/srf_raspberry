########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = g++
# CXXFLAGS = -std=c++17 -Wall -g -fsanitize=address
CXXFLAGS = -std=c++17 -Wall -g
# LDFLAGS = -L/usr/local/lib -L/home/mchu/instaladores/dlib/build/dlib/libdlib.a -ldlib -ljpeg -lwebp -lpng -lblas -llapack -lmariadb -lv4l2 -lcamera -lcamera-base -lpthread -lcamera_app -lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_highgui -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_intensity_transform -lopencv_line_descriptor -lopencv_quality -lopencv_rapid -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_videostab -lopencv_videoio -lopencv_xfeatures2d -lopencv_shape -lopencv_ml -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core -pthread -lcrypto -lssl /home/fractal/instaladores/tensorflow/tensorflow/lite/tools/make/gen/linux_aarch64/lib/libtensorflow-lite.a /home/fractal/instaladores/tensorflow/tensorflow/lite/tools/make/downloads/flatbuffers/build/libflatbuffers.a /lib/aarch64-linux-gnu/libdl.so.2
LDFLAGS = -L/usr/local/lib -lhailort -ljpeg -lwebp -lpng -lblas -llapack -lv4l2 -lcamera -lcamera-base -lpthread -lcamera_app -lopencv_highgui -lopencv_dnn -lopencv_videoio -lopencv_ml -lopencv_video -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_photo -lopencv_imgproc -lopencv_core -pthread -lcrypto -lssl -lopencv_tracking /lib/aarch64-linux-gnu/libdl.so.2


# Makefile settings - Can be customized.
APPNAME = myapp

EXT = .cpp
SRCDIR = src/app
OBJDIR = obj/app
DEPDIR = dep/app
LIBDIR = obj/lib

DIROBJBASE = ./obj
DIROBJLIBS = ./obj/lib
DIROBJAPP = ./obj/app
DIROBJTESTS = ./obj/test

# incluir todos los archivos de los que se tiene dependencia
# INCLUDES =-I./include -I/usr/include/hailo  -I/usr/include/libcamera -I/usr/local/include/opencv4 -I/home/fractal/instaladores/tensorflow/tensorflow/lite/tools/make/downloads/flatbuffers/include -I/home/fractal/instaladores/tensorflow -I/home/fractal/tensorflow/tensorflow/lite/tools/make/downloads/absl 
INCLUDES =-I./include -I/usr/include/hailo  -I/usr/include/libcamera -I/usr/local/include/opencv4

# Referencia a todos los archivos de la carpeta lib
LIBOBJ = $(wildcard $(LIBDIR)/*/*.o)

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
OBJTEST = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP = $(OBJ:$(OBJDIR)/%.o=$(DEPDIR)%.d)

# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(OBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

########################################################################
####################### Targets beginning here #########################
########################################################################

all: $(DIROBJBASE) $(DIROBJAPP) $(DIROBJLIBS) $(APPNAME)

# Builds the app
$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $(LIBOBJ) $^ $(LDFLAGS)

# Creates the dependecy rules
%.d: $(SRCDIR)/%$(EXT)
	@$(CPP) $(CFLAGS) $< -MM -MT $($(DEPDIR)@:%.d=$(OBJDIR)/%.o) >$@

# Includes all .h files
-include $(DEP)

# Building rule for .o files and its .c/.cpp in combination with all .h
# $(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) 
#libgeneral libutils libgraphics libweb libdeeplearning libdatabase
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) libs 
	$(CC) $(CXXFLAGS) -o $@ -c $< $(INCLUDES)

# Crea el directorio de todos los archivos objeto
$(DIROBJBASE):
	mkdir $(DIROBJBASE)

# Crea el directorio de todos los archivos objeto para la aplicacion
$(DIROBJAPP):	
	mkdir $(DIROBJAPP)

# Crea el directorio base de todos los archivos objeto de las librerias
$(DIROBJLIBS):
	mkdir $(DIROBJLIBS)

# Crea el directorio base de todos los archivos objeto de las aplicaciones de testing
$(DIROBJTESTS):
	mkdir $(DIROBJTESTS)

##############################################################################
################## Compiles the testing application ##########################
##############################################################################
#.PHONY: testDatabase
#testDatabase: libs  $(DIROBJTESTS) 
#	$(MAKE) -C ./src/test/database


################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	rm -r $(DIROBJBASE)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)

#################### Cleaning rules for Windows OS #####################
# Cleans complete project
.PHONY: cleanw
cleanw:
	$(DEL) $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)

# Cleans only all files with the extension .d
.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)


# Compila sololas librerias
.PHONY: libs
# libs: $(DIROBJBASE) $(DIROBJLIBS) libgeneral libutils libgraphics libweb libdeeplearning libfaceredlib libdatabase libfacerec
libs: $(DIROBJBASE) $(DIROBJLIBS) libgeneral libutils libgraphics  libweb hailo8l



#################### Compila cada carpeta de cada libreria por individual #####################
.PHONY: libgeneral
libgeneral:
	$(MAKE) -C ./src/lib/general

.PHONY: libfacerec
libfacerec:
	$(MAKE) -C ./src/lib/facerec

.PHONY: libutils
libutils:
	$(MAKE) -C ./src/lib/utils

.PHONY: libgraphics
libgraphics:
	$(MAKE) -C ./src/lib/graphics

.PHONY: libweb
libweb:
	$(MAKE) -C ./src/lib/web

.PHONY: libdeeplearning
libdeeplearning:
	$(MAKE) -C ./src/lib/deeplearning

.PHONY: libdatabase
libdatabase:
	$(MAKE) -C ./src/lib/database


.PHONY: hailo8l
hailo8l:
	$(MAKE) -C ./src/lib/hailolib

.PHONY: cleanLibDatabase
cleanLibDatabase:
	rm -r ./obj/lib/database

#################### Compila/Borra cada aplicacion de test #####################
.PHONY: testDatabase
testDatabase: libs
	$(MAKE) -C ./src/test/database

.PHONY: cleanTestDatabase
cleanTestDatabase:
	rm -r ./obj/test/database
