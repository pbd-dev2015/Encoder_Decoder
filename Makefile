#http://stackoverflow.com/questions/7004702/how-can-i-create-a-makefile-for-c-projects-with-src-obj-and-bin-subdirectories

TARGET = endec

SRCDIR	= code/src
INCDIR	= code/include
OBJDIR	= obj
BINDIR	= bin

CC = gcc

CFLAGS = -g -Wall -I$(INCDIR)

LINKER = $(CC) -o

SOURCES	:= $(wildcard $(SRCDIR)/*.c)
INCLUDES	:= $(wildcard $(INCDIR)/*.h)
OBJECTS	:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
RM	= rm -rf

# typing 'make' will invoke the first target entry in the file 
#all: $(TARGET)

#$@ will give the name of the current target which is $(BINDIR)/$(TARGET). $@: the name of the target of the rule
# For example, the following does the same as what the .
# program: program.o mylib.o	=> $(OBJECTS)
#	gcc -o program program.o mylib.o	=> $@ is $(BINDIR)/$(TARGET) which is endec. In the example, it is program.
$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(OBJECTS)
	@echo "Linking Complete."

#$<: the name of the prerequisite of the rule
# For example, we do gcc -g -Wall -c helloworld.c -o helloworld.o
# for "| $(OBJDIR)", see http://stackoverflow.com/a/6170280. It is to create the bin and obj directories before actually compiling.
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully."

#we don't use mkdir -p commands because the official documentation says so. http://www.gnu.org/s/hello/manual/make/Utilities-in-Makefiles.html and http://stackoverflow.com/questions/99132/how-to-prevent-directory-already-exists-error-in-a-makefile-when-using-mkdir#comment10617472_99174
$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)
	test -d $(BINDIR) || mkdir $(BINDIR)

clean:
	@$(RM) $(BINDIR)
	@$(RM) $(OBJDIR)
