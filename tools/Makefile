CC		:= gcc

ELFCONF	:= elfconf

all: $(ELFCONF)
	@:

$(ELFCONF): $(ELFCONF).c
	@$(CC) -o $@ $<
	@echo "  HOSTCC  $(ELFCONF)"

.PHONY: clean
clean:
	@rm -f $(ELFCONF)
	@echo "  CLEAN   $(ELFCONF)"
