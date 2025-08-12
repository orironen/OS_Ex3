CXX = gcc
SUBDIRS = level1 level2 level3 level4 level6 level7 level9 level10

.PHONY: all coverage clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

coverage:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir coverage; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
