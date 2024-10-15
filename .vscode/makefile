#source file search paths
VPATH = ./src ../PathFinder/src ../raven-set

	# compiler include search paths
INCS = -I./include \
	-I../windex/include \
	-I../raven-set \
	-I../PathFinder/src \
	-I../../code_ext/boost/boost1_78 

LIBS = \
	-lgdiplus -lgdi32 -lcomdlg32 -lstdc++fs \
	#-mwindows

static_link_flags = -static-libstdc++ -static-libgcc -static -lComctl32

# folders
ODIR = ./.vscode/obj

OPT = -O2
CCflags = -std=c++17 $(OPT) -D_mingw_

# sources
OBJ_starter = \
	$(ODIR)/main.o \
	$(ODIR)/cGUI.o \
	$(ODIR)/cGraph.o \
	$(ODIR)/GraphTheory.o \

OBJ_search = \
	$(ODIR)/search.o \
	$(ODIR)/cssex.o \
	$(ODIR)/cRunWatch.o \

### COMPILE

$(ODIR)/%.o: %.cpp  
	g++ $< \
	-c -o $@  $(INCS) $(CCflags)

starter: $(OBJ_starter)
	g++ $(OPT) \
	-o bin/cratePusher.exe $^  \
	$(static_link_flags) $(LIBS)

search: $(OBJ_search)
	g++ $(OPT) \
	-o bin/searcher.exe $^  \
	$(static_link_flags) $(LIBS)

.PHONY: clean
clean:
	del /q .vscode\\obj\\*.o

