#ifndef SHOTGUN_XML_H
#define SHOTGUN_XML_H

#include "shotgun_private.h"

#define XML_STARTTLS "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>"

#ifdef __cplusplus
extern "C" {
#endif

char *xml_stream_init_create(const char *from, const char *to, const char *lang, size_t *len);
Eina_Bool xml_stream_init_read(Shotgun_Auth *auth, char *xml, size_t size);
Eina_Bool xml_starttls_read(char *xml, size_t size);
char *xml_sasl_write(const char *sasl, size_t *len);
Eina_Bool xml_sasl_read(const unsigned char *xml, size_t size __UNUSED__);

char *xml_iq_write(Shotgun_Auth *auth, Shotgun_Iq_Preset p, size_t *len);
Shotgun_Event_Iq *xml_iq_read(Shotgun_Auth *auth, char *xml, size_t size);

char *xml_message_write(Shotgun_Auth *auth, const char *to, const char *msg, size_t *len);
Shotgun_Event_Message *xml_message_read(Shotgun_Auth *auth, char *xml, size_t size);


char *xml_presence_write(Shotgun_Auth *auth, Shotgun_User_Status st, const char *msg, size_t *len);
Shotgun_Event_Presence *xml_presence_read(Shotgun_Auth *auth, char *xml, size_t size);

#ifdef __cplusplus
}
#endif

#endif
