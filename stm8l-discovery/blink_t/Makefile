
SOURCES = main.c
PLATFORM = stm8s105c6
PROGRAMMER = stlink
DIST = dist
TARGET = $(DIST)/$(notdir $(CURDIR)).hex
OBJECTS = $(DIST)/$(SOURCES:.c=.rel)

$(TARGET): $(OBJECTS)
	sdcc -lstm8 -mstm8 --opt-code-size $< -o $(TARGET)

$(OBJECTS): $(SOURCES)
	mkdir -p $(DIST)
	sdcc -lstm8 -mstm8 --opt-code-size -c $< -o $@

flash: $(TARGET)
	stm8flash -c $(PROGRAMMER) -p $(PLATFORM) -w $(TARGET)

clean:
	rm -rf $(DIST)


.PHONY: $(DIST) flash clean
