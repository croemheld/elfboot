BASH := bash

TOOLCHAIN_GENERATOR := gentoolchain.sh

.PHONY: all
all: $(TOOLCHAIN_GENERATOR)
	$(BASH) $(TOOLCHAIN_GENERATOR) $(TARGET)

.PHONY: clean
clean:
	rm -rf cross