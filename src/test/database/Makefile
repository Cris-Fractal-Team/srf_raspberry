########################################################################
# No compilar directamente, se debe llamar desde el MAKE del proyecto
# pues tiene dependencias de las librerias de la aplicacion

# Este make se ejecuta usando: make testDatabase

# Revisar al final del archivo Makefile la definicion de ".PHONY: testdatabase"
########################################################################

# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -std=c++17 -Wall -g
LDFLAGS = -L/usr/local/lib -lv4l2  -lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_highgui -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_intensity_transform -lopencv_line_descriptor -lopencv_quality -lopencv_rapid -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_videostab -lopencv_videoio -lopencv_xfeatures2d -lopencv_shape -lopencv_ml -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core -pthread -lcrypto -lssl /lib/aarch64-linux-gnu/libdl.so.2

# Cambiar el nombre "testDatabase" por el de la aplicacion de test que se desee generar
APPNAME = ../../../testDatabase

EXT = .cpp
SRCDIR = .

# Cambiar el directorio "database" por el de la aplicacion de test que se desee compilar, son 3 opciones OBJDIR, DEPDIR y DIROBJAPP
OBJDIR = ../../../obj/test/database
DEPDIR = ../../../dep/test/database
LIBDIR = ../../../obj/lib

DIROBJBASE = ../../../obj
DIROBJLIBS = ../../../obj/lib
DIROBJTEST = ../../../obj/test
DIROBJAPP = ../../../obj/test/database

# incluir todos los archivos de los que se tiene dependencia 
# inclue -I/usr/include/mariadb
INCLUDES =-I../../../include  -I/usr/local/include/opencv4 -I/home/mchu/instladores/tensorflow/tensorflow/lite/tools/make/downloads/flatbuffers/include -I/home/fractal/instaladores/tensorflow -I/home/mchu/tensorflow/tensorflow/lite/tools/make/downloads/absl 

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

all: $(DIROBJBASE) $(DIROBJTEST) $(DIROBJAPP) $(DIROBJLIBS) $(APPNAME)

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
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) 
	$(CC) $(CXXFLAGS) -o $@ -c $< $(INCLUDES)

# Crea el directorio de todos los archivos objeto
$(DIROBJBASE):
	mkdir $(DIROBJBASE)

# Crea el directorio de todas las aplicaciones de test
$(DIROBJTEST):
	mkdir $(DIROBJTEST)


# Crea el directorio de todos los archivos objeto para la aplicacion
$(DIROBJAPP):		
	mkdir $(DIROBJAPP)

# Crea el directorio base de todos los archivos objeto de las librerias
$(DIROBJLIBS):
	mkdir $(DIROBJLIBS)


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

