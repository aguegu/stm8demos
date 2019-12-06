UID = $(shell id -u)
GID = $(shell id -g)
PROGRAMMER = stlinkv2
PARTNO = stm8s103f3

dist:
	docker run -t --rm -v ${CURDIR}:/root/workspace aguegu/sdcc sh -c "\
		make -f sdcc.Makefile \
		&& chown $(UID):$(GID) -R dist \
	"

clean:
	rm -rf dist/

flash:
	stm8flash -c $(PROGRAMMER) -p $(PARTNO) -w dist/ch.hex

all: dist flash

.PHONY: dist clean flash all
