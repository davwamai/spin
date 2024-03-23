APP = cnmr-driver
LDLIBS = -lm

APP_OBJS = cnmr-driver.o

all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CC) -o $@ $(APP_OBJS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(APP) *.o

