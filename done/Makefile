## ======================================================================

GTK_INCLUDE := `pkg-config --cflags gtk+-3.0`
GTK_LIBS := `pkg-config --libs gtk+-3.0`

.PHONY: clean new style feedback submit1 submit2 submit

CFLAGS += -std=c11 -Wall -pedantic -g -D_DEFAULT_SOURCE

# a bit more checks if you'd like to (uncomment
CFLAGS += -Wextra -Wfloat-equal -Wshadow                         \
-Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings \
-Wunreachable-code -Wconversion

# uncomment if you want to add DEBUG flag
# CPPFLAGS += -DDEBUG

 CPPFLAGS += -DBLARGG

# uncomment if you want to play tetris
# CPPFLAGS += -DNO_ROM_WRITE

# ----------------------------------------------------------------------

# all those libs are required on Debian, feel free to adapt it to your box
LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

final:: unit-test-bit unit-test-alu unit-test-bus unit-test-component unit-test-memory unit-test-cpu unit-test-cpu-dispatch-week08 unit-test-cpu-dispatch-week09 unit-test-timer unit-test-cartridge unit-test-cpu-dispatch unit-test-alu_ext unit-test-bit-vector test-cpu-week08 test-cpu-week09 test-gameboy gbsimulator

all:: 

TARGETS := test-cpu-week08 test-cpu-week09 test-gameboy gbsimulator
CHECK_TARGETS := unit-test-bit unit-test-alu unit-test-bus unit-test-component unit-test-memory unit-test-cpu unit-test-cpu-dispatch-week08 unit-test-cpu-dispatch-week09 unit-test-timer unit-test-cartridge unit-test-cpu-dispatch unit-test-alu_ext unit-test-bit-vector
OBJS = 
OBJS_NO_STATIC_TESTS =
OBJS_STATIC_TESTS = 
OBJS = $(OBJS_STATIC_TESTS) $(OBJS_NO_STATIC_TESTS)

unit-test-bit: unit-test-bit.o bit.o

unit-test-alu: unit-test-alu.o component.o alu.o bit.o error.o memory.o

unit-test-bus: unit-test-bus.o component.o bit.o bus.o memory.o

unit-test-component: unit-test-component.o component.o bus.o memory.o bit.o

unit-test-memory: unit-test-memory.o bus.o memory.o component.o bit.o

unit-test-cpu: unit-test-cpu.o error.o alu.o bit.o util.o cpu.o bus.o memory.o component.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o image.o bit_vector.o
	gcc -L . unit-test-cpu.o error.o alu.o bit.o util.o cpu.o bus.o memory.o component.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o image.o bit_vector.o -lcs212gbfinalext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cpu

unit-test-cpu-dispatch-week08: unit-test-cpu-dispatch-week08.o error.o alu.o opcode.o bit.o gameboy.o util.o bus.o memory.o component.o cpu-alu.o cpu-registers.o cpu-storage.o bootrom.o cartridge.o timer.o image.o bit_vector.o
	gcc -L . unit-test-cpu-dispatch-week08.o error.o alu.o opcode.o bit.o gameboy.o util.o bus.o memory.o component.o cpu-alu.o cpu-registers.o cpu-storage.o bootrom.o cartridge.o timer.o image.o bit_vector.o -lcs212gbfinalext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cpu-dispatch-week08

unit-test-cpu-dispatch-week09: unit-test-cpu-dispatch-week09.o error.o alu.o bit.o bus.o memory.o component.o opcode.o util.o cpu-alu.o cpu-registers.o cpu-storage.o image.o bit_vector.o
	gcc -L . unit-test-cpu-dispatch-week09.o error.o alu.o bit.o bus.o memory.o component.o opcode.o util.o cpu-alu.o cpu-registers.o cpu-storage.o image.o bit_vector.o -lcs212gbfinalext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cpu-dispatch-week09

test-cpu-week08: test-cpu-week08.o opcode.o bit.o cpu.o cpu-alu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o util.o error.o
	gcc -L . test-cpu-week08.o opcode.o bit.o cpu.o cpu-alu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o util.o error.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o test-cpu-week08

test-cpu-week09: test-cpu-week09.o opcode.o bit.o cpu.o cpu-alu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o util.o error.o
	gcc -L . test-cpu-week09.o opcode.o bit.o cpu.o cpu-alu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o util.o error.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o test-cpu-week09


gbsimulator: LDFLAGS += -L.
gbsimulator: LDLIBS += -lcs212gbfinalext
gbsimulator: LDLIBS += $(GTK_LIBS)
gbsimulator: CFLAGS += $(GTK_INCLUDE)
gbsimulator: gbsimulator.o sidlib.o cpu.o alu.o bit.o image.o bus.o memory.o component.o bit_vector.o gameboy.o timer.o cartridge.o bootrom.o opcode.o cpu-registers.o cpu-storage.o cpu-alu.o

test-gameboy: LDFLAGS += -L.
test-gameboy: LDLIBS += -lcs212gbfinalext
test-gameboy: CPPFLAGS += -DBLARGG
test-gameboy: LDLIBS += $(GTK_LIBS)
test-gameboy: CFLAGS += $(GTK_INCLUDE)
test-gameboy: test-gameboy.o gameboy.o bus.o memory.o component.o cpu.o alu.o bit.o timer.o cartridge.o util.o error.o bootrom.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o image.o bit_vector.o sidlib.o

unit-test-cartridge: unit-test-cartridge.o error.o cartridge.o component.o memory.o bus.o cpu.o alu.o bit.o cpu-registers.o cpu-alu.o cpu-storage.o opcode.o image.o bit_vector.o
	gcc -L . unit-test-cartridge.o error.o cartridge.o component.o memory.o bus.o cpu.o alu.o bit.o cpu-registers.o cpu-alu.o cpu-storage.o opcode.o image.o bit_vector.o -lcs212gbfinalext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cartridge

unit-test-timer: unit-test-timer.o util.o error.o timer.o component.o memory.o bit.o cpu.o alu.o bus.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o image.o bit_vector.o
	gcc -L . unit-test-timer.o util.o error.o timer.o component.o memory.o bit.o cpu.o alu.o bus.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o image.o bit_vector.o -lcs212gbfinalext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-timer

unit-test-alu_ext: unit-test-alu_ext.o cpu-storage.o cpu-registers.o cpu-alu.o alu.o bus.o bit.o error.o image.o bit_vector.o
	gcc -L . unit-test-alu_ext.o cpu-storage.o cpu-registers.o cpu-alu.o alu.o bus.o bit.o error.o image.o bit_vector.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-alu_ext

unit-test-cpu-dispatch: unit-test-cpu-dispatch.o cpu-storage.o cpu-registers.o cpu-alu.o opcode.o alu.o component.o memory.o bus.o bit.o error.o image.o bit_vector.o
	gcc -L . unit-test-cpu-dispatch.o cpu-storage.o cpu-registers.o cpu-alu.o opcode.o alu.o component.o memory.o bus.o bit.o error.o image.o bit_vector.o -lcs212gbfinalext  -lcheck -lm -lrt -pthread -lsubunit  -o unit-test-cpu-dispatch

unit-test-bit-vector: unit-test-bit-vector.o bit_vector.o error.o bit.o

alu.o: alu.c bit.h alu.h error.h
bit.o: bit.c bit.h error.h
bit_vector.o: bit_vector.c bit_vector.h bit.h
bootrom.o: bootrom.c error.h bootrom.h bus.h memory.h component.h \
 gameboy.h cpu.h alu.h bit.h timer.h cartridge.h
bus.o: bus.c bus.h memory.h component.h error.h bit.h
cartridge.o: cartridge.c cartridge.h component.h memory.h bus.h error.h
component.o: component.c error.h memory.h component.h
cpu-alu.o: cpu-alu.c error.h bit.h alu.h cpu-alu.h opcode.h cpu.h bus.h \
 memory.h component.h cpu-storage.h cpu-registers.h
cpu.o: cpu.c error.h opcode.h bit.h cpu.h alu.h bus.h memory.h \
 component.h cpu-alu.h cpu-registers.h cpu-storage.h util.h
cpu-registers.o: cpu-registers.c cpu.h alu.h bit.h bus.h memory.h \
 component.h cpu-registers.h
cpu-storage.o: cpu-storage.c error.h cpu-storage.h memory.h opcode.h \
 bit.h cpu.h alu.h bus.h component.h cpu-registers.h gameboy.h timer.h \
 cartridge.h util.h
error.o: error.c
gameboy.o: gameboy.c gameboy.h bus.h memory.h component.h cpu.h alu.h \
 bit.h timer.h cartridge.h error.h bootrom.h joypad.h lcdc.h
gbsimulator.o: gbsimulator.c sidlib.h lcdc.h cpu.h alu.h bit.h bus.h \
 memory.h component.h image.h bit_vector.h gameboy.h timer.h cartridge.h \
 joypad.h
image.o: image.c error.h image.h bit_vector.h bit.h
libsid_demo.o: libsid_demo.c sidlib.h
memory.o: memory.c error.h memory.h
opcode.o: opcode.c opcode.h bit.h
sidlib.o: sidlib.c sidlib.h
test-cpu-week08.o: test-cpu-week08.c opcode.h bit.h cpu.h alu.h bus.h \
 memory.h component.h cpu-storage.h util.h error.h
test-cpu-week09.o: test-cpu-week09.c opcode.h bit.h cpu.h alu.h bus.h \
 memory.h component.h cpu-storage.h util.h error.h
test-gameboy.o: test-gameboy.c gameboy.h bus.h memory.h component.h cpu.h \
 alu.h bit.h timer.h cartridge.h util.h error.h
timer.o: timer.c timer.h component.h memory.h bit.h cpu.h alu.h bus.h \
 error.h util.h
unit-test-alu.o: unit-test-alu.c tests.h error.h alu.h bit.h
unit-test-bit.o: unit-test-bit.c tests.h error.h bit.h
unit-test-bus.o: unit-test-bus.c tests.h error.h bus.h memory.h \
 component.h util.h
unit-test-bit-vector.o: unit-test-bit-vector.c tests.h error.h \
 bit_vector.h bit.h image.h
unit-test-cartridge.o: unit-test-cartridge.c tests.h error.h cartridge.h \
 component.h memory.h bus.h cpu.h alu.h bit.h
unit-test-component.o: unit-test-component.c tests.h error.h bus.h \
 memory.h component.h
unit-test-cpu.o: unit-test-cpu.c tests.h error.h alu.h bit.h opcode.h \
 util.h cpu.h bus.h memory.h component.h cpu-registers.h cpu-storage.h \
 cpu-alu.h
unit-test-cpu-dispatch-week08.o: unit-test-cpu-dispatch-week08.c tests.h \
 error.h alu.h bit.h cpu.h bus.h memory.h component.h opcode.h gameboy.h \
 timer.h cartridge.h util.h unit-test-cpu-dispatch.h cpu.c cpu-alu.h \
 cpu-registers.h cpu-storage.h
unit-test-cpu-dispatch-week09.o: unit-test-cpu-dispatch-week09.c tests.h \
 error.h alu.h bit.h cpu.h bus.h memory.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h
unit-test-memory.o: unit-test-memory.c tests.h error.h bus.h memory.h \
 component.h
unit-test-timer.o: unit-test-timer.c util.h tests.h error.h timer.h \
 component.h memory.h bit.h cpu.h alu.h bus.h
unit-test-alu_ext.o: unit-test-alu_ext.c tests.h error.h alu.h bit.h \
 alu_ext.h
unit-test-cpu-dispatch.o: unit-test-cpu-dispatch.c tests.h error.h alu.h \
 bit.h cpu.h bus.h memory.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h
util.o: util.c


# ----------------------------------------------------------------------


clean::
	-@/bin/rm -f *.o *~ $(CHECK_TARGETS)

new: clean all

static-check:
	scan-build -analyze-headers --status-bugs -maxloop 64 make CC=clang new

style:
	astyle -n -o -A8 -xt0 *.[ch]

# all those libs are required on Debian, adapt to your box
$(CHECK_TARGETS): LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

check:: $(CHECK_TARGETS)
	$(foreach target,$(CHECK_TARGETS),./$(target) &&) true

# target to run tests
check:: all
	@if ls tests/*.*.sh 1> /dev/null 2>&1; then \
	  for file in tests/*.*.sh; do [ -x $$file ] || echo "Launching $$file"; ./$$file || exit 1; done; \
	fi


libsid.so: sidlib.c
	$(CC) -fPIC -shared $(CPPFLAGS) $(CFLAGS) $(GTK_INCLUDE) $^ -o $@

libsid_demo.o: CFLAGS += $(GTK_INCLUDE)

libsid_demo: LDFLAGS += -L.
libsid_demo: LDLIBS += $(GTK_LIBS) -lsid
libsid_demo: libsid_demo.o libsid.so
