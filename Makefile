
SRCDIR = .
OBJDIR = .
INCDIR_PRIV = ./include

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

CFLAGS += -Werror -Wall -Wextra \
          -Wformat=2 -Wshadow \
          -Wwrite-strings -Wredundant-decls \
          -Wpedantic -Wmissing-declarations -Wno-attributes \
          -Wno-format-nonliteral \
          -g3 -I$(INCDIR_PRIV)

CFLAGS += -g3 -I$(INCDIR_PRIV)


# targets
all: onu_agent

onu_agent: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

$(OBJDIR)/:
	$(MKDIR) -p $@

clean:
	rm -rf $(OUTPUTDIR) onu_agent

.PHONY: all clean
