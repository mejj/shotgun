AM_CFLAGS = \
  @AZY_CFLAGS@ \
  @ELM_CFLAGS@ \
  @edbus_CFLAGS@ \
  @enotify_CFLAGS@ \
 -DDATA_DIR=\"$(datadir)\" \
 -DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
 -DPACKAGE_LIB_DIR=\"$(libdir)\" \
 -DPACKAGE_SRC_DIR=\"$(top_srcdir)\" \
 -I@top_builddir@ \
 -I@top_srcdir@/src/include

AM_CXXFLAGS = $(AM_CFLAGS)

bin_PROGRAMS = shotgun

shotgun_LDADD = \
  @AZY_LIBS@ \
  @ELM_LIBS@ \
  @edbus_LIBS@ \
  @enotify_LIBS@ \
  -lm \
  @top_builddir@/src/lib/libshotgun.la

shotgun_SOURCES = \
azy.c \
chat.c \
chat_image.c \
contact.c \
contact_list.c \
dbus.c \
eet.c \
events.c \
getpass_x.c \
login.c \
logging.c \
main.c \
settings.c \
sha1.c \
sha1.h \
sha1wrap.c \
ui.c \
ui.h \
util.c

MAINTAINERCLEANFILES = Makefile.in
