# gslsl building

GLSLC = glslc
CFLAGS = -g

all: options vert.spv geom.spv frag.spv

options:
	@echo glsl build options:
	@echo "GLSLC  = $(GLSLC)"
	@echo "CFLAGS = $(CFLAGS)"

vert.spv: shader.vert
	$(GLSLC) $(CFLAGS) $< -o $@

geom.spv: shader.geom
	$(GLSLC) $(CFLAGS) $< -o $@

frag.spv: shader.frag
	$(GLSLC) $(CFLAGS) $< -o $@

clean:
	rm -f vert.spv geom.spv frag.spv
