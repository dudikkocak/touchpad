TARGET = touchpad.exe
OBJ = main.obj
CFLAGS = /O1 /MT /GS- /DNDEBUG /c
LFLAGS = /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup

all: $(TARGET)

$(TARGET): $(OBJ)
    link $(OBJ) user32.lib advapi32.lib $(LFLAGS) /OUT:$(TARGET)

$(OBJ): main.cpp
    cl $(CFLAGS) main.cpp

clean:
    del $(OBJ) $(TARGET)