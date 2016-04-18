#http://stackoverflow.com/questions/7004702/how-can-i-create-a-makefile-for-c-projects-with-src-obj-and-bin-subdirectories

TARGET = endec

SRCDIR	= code/src
INCDIR	= code/include
OBJDIR	= bin
BINDIR	= bin

CC = gcc

CFLAGS = -g -Wall -I$(INCDIR)

LINKER = $(CC) -o

SOURCES	:= $(wildcard $(SRCDIR)/*.c)
INCLUDES	:= $(wildcard $(INCDIR)/*.h)
OBJECTS	:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
RM	= rm -f

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
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully."

clean:
	@$(RM) $(BINDIR)/$(TARGET)
	@$(RM) $(OBJECTS)
