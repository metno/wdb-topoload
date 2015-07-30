man_MANS = wdb-topoload.1

wdb-topoload.1:	doc/wdb-topoload.man.xml doc/docbook/wdb_entities.ent
	$(DOCBOOK) man $<
	
EXTRA_DIST += doc/wdb-topoload.man.xml \
	doc/docbook/wdb_entities.ent \
	doc/docbook/xinclude.mod
	
CLEANFILES += wdb-topoload.1
