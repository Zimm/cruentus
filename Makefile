CC=gcc
CFLAGS=-Wall -Werror -Wextra
LDFLAGS=
OBJ=cruentus.o
OBJDIR=obj
all: cruentus

.PHONY: all clean cruentus $(OBJDIR)

cruentus: $(OBJDIR)/cruentus

$(OBJDIR)/cruentus: $(addprefix $(OBJDIR)/,$(OBJ))
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
