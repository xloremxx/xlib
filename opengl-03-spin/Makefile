LIBS += -lX11
LIBS += -lGL
LIBS += -lGLU

all: app

app: main.o
	$(CC) -o $@ $^ $(LIBS)
