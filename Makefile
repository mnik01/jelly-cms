CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -O2
TARGET=jelly-cms
SOURCE=main.c

# Default target
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -rf build

# Install (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Test build (run build command)
test: $(TARGET)
	./$(TARGET) build

# Help
help:
	@echo "Available targets:"
	@echo "  $(TARGET)    - Build the jelly-cms executable"
	@echo "  clean       - Remove build artifacts and executable"
	@echo "  install     - Install jelly-cms to /usr/local/bin"
	@echo "  uninstall   - Remove jelly-cms from /usr/local/bin"
	@echo "  test        - Build and run jelly-cms build command"
	@echo "  help        - Show this help message"

.PHONY: clean install uninstall test help