TOP=..

include $(TOP)/Makefile.inc

COMMON_FLAGS = -I$(TOP)/lib -I$(TOP)/include_$(GCABI) -g -std=c99 -fPIC  -I../resources  -I../include
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS) 
LDFLAGS += -lm

TARGETS = fbtest etna_test rotate_cube cube_companion mip_cube rstests mip_cube_state
COMPANION_OBJS = ../resources/companion_array.o ../resources/companion_mesh.o ../resources/companion_texture.o

all: $(TARGETS)

clean:
	rm -f *.o ../lib/*.o
	rm -f $(TARGETS)

fbtest: fbtest.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna_fb.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

etna_test: etna_test.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna.o ../lib/etna_rs.o ../lib/etna_fb.o ../lib/etna_mem.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

rotate_cube: rotate_cube.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna.o ../lib/etna_rs.o ../lib/etna_fb.o ../lib/etna_mem.o ../lib/esTransform.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

mip_cube: mip_cube.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna.o ../lib/etna_rs.o ../lib/etna_fb.o ../lib/etna_mem.o ../lib/esTransform.o ../lib/dds.o ../lib/etna_bswap.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

mip_cube_state: mip_cube_state.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna.o ../lib/etna_rs.o ../lib/etna_fb.o ../lib/etna_mem.o ../lib/esTransform.o ../lib/dds.o ../lib/etna_bswap.o
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

cube_companion: cube_companion.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna.o ../lib/etna_rs.o ../lib/etna_fb.o ../lib/etna_mem.o ../lib/esTransform.o $(COMPANION_OBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

rstests: rstests.o ../lib/write_bmp.o ../lib/viv.o ../lib/etna.o ../lib/etna_rs.o ../lib/etna_fb.o ../lib/etna_mem.o ../lib/esTransform.o $(COMPANION_OBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)

