#ifndef __UI_H
#define __UI_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef alloca
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#elif defined __GNUC__
#define alloca __builtin_alloca
#elif defined _AIX
#define alloca __alloca
#else
#include <stddef.h>
void *alloca (size_t);
#endif
#endif

#include <Shotgun.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <Elementary.h>
#ifdef HAVE_ECORE_X
# include <Ecore_X.h>
#endif
#ifdef HAVE_DBUS
# include <E_DBus.h>
#endif
#ifdef HAVE_AZY
# include <Azy.h>
#endif

#ifndef __UNUSED__
# define __UNUSED__ __attribute__((unused))
#endif

#ifndef strdupa
# define strdupa(str)       strcpy(alloca(strlen(str) + 1), str)
#endif

#ifndef strndupa
# define strndupa(str, len) strncpy(alloca(len + 1), str, len)
#endif

#define DBG(...)            EINA_LOG_DOM_DBG(ui_log_dom, __VA_ARGS__)
#define INF(...)            EINA_LOG_DOM_INFO(ui_log_dom, __VA_ARGS__)
#define WRN(...)            EINA_LOG_DOM_WARN(ui_log_dom, __VA_ARGS__)
#define ERR(...)            EINA_LOG_DOM_ERR(ui_log_dom, __VA_ARGS__)
#define CRI(...)            EINA_LOG_DOM_CRIT(ui_log_dom, __VA_ARGS__)

#define WEIGHT evas_object_size_hint_weight_set
#define ALIGN evas_object_size_hint_align_set
#define EXPAND(X) WEIGHT((X), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
#define FILL(X) ALIGN((X), EVAS_HINT_FILL, EVAS_HINT_FILL)

#define IF_NOT_ILLUME if ((!cl->settings.enable_illume) && (!cl->illume_box))
#define IF_ILLUME if (cl->settings.enable_illume && cl->illume_box)

extern int ui_log_dom;

typedef struct Contact_List Contact_List;
typedef struct Contact Contact;

typedef void (*Contact_List_Item_Tooltip_Cb)(void *item, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);
typedef Eina_Bool (*Contact_List_Item_Tooltip_Resize_Cb)(void *item, Eina_Bool set);
typedef void *(*Contact_List_At_XY_Item_Get)(void *list, Evas_Coord x, Evas_Coord y, int *ret);

typedef struct Shotgun_Settings
{
   Eina_Bool disable_notify;
   Eina_Bool enable_chat_focus;
   Eina_Bool enable_chat_promote;
   Eina_Bool enable_chat_newselect;
   Eina_Bool enable_account_info;
   Eina_Bool enable_last_account;
   Eina_Bool enable_logging;
   Eina_Bool enable_illume;
   Eina_Bool disable_image_fetch;
   Eina_Bool disable_reconnect;
   Eina_Bool enable_presence_save;
   unsigned int allowed_image_age;
   unsigned int allowed_image_size;
} Shotgun_Settings;

struct Contact_List
{
   Evas_Object *win; /* window */
   Evas_Object *illume_box; /* horizontal box used in illume mode */
   Evas_Object *illume_frame; /* list frame used in illume mode */
   Evas_Object *flip; /* flip for settings */
   Evas_Object *box; /* main box */
   Evas_Object *settings_box; /* settings box */
   Evas_Object *list; /* list/grid object */
   Evas_Object *status_entry; /* entry for user's status */

   Evas_Object *pager; /* pager for user add wizard */
   Eina_List *pager_entries; /* entry in pager */
   Eina_Bool pager_state : 1; /* 0 for first page, 1 for second */

   Eina_List *users_list; /* list of all contacts */
   Eina_List *chat_wins; /* list of all chat windows */
   Eina_Hash *users; /* hash of jid<->Contact */
   Eina_Hash *images; /* hash of img_url<->Image */
   Eina_List *image_list; /* list of Images sorted by timestamp */
   size_t image_size; /* current total size of images in memory (in bytes) */
   Ecore_Timer *status_timer; /* timer for sending text in status_entry */

   int alert_colors[3]; /* array of r/g/b for contact->animator */

   Eina_Bool mode : 1; /* 0 for list, 1 for grid */
   Eina_Bool view : 1; /* 0 for regular, 1 for offlines */
#ifdef HAVE_DBUS
   E_DBus_Connection *dbus;
   E_DBus_Object *dbus_object;
#endif

   Shotgun_Settings settings;
   Ecore_Idler *image_cleaner;
   Ecore_Timer *logs_refresh;

   /* fps for doing stuff to both list and grid views with the same function */
   Ecore_Data_Cb list_item_parent_get[2];
   Ecore_Data_Cb list_selected_item_get[2];
   Contact_List_At_XY_Item_Get list_at_xy_item_get[2];
   Ecore_Cb list_item_del[2];
   Ecore_Cb list_item_update[2];
   Ecore_Cb list_item_promote[2];
   Contact_List_Item_Tooltip_Cb list_item_tooltip_add[2];
   Contact_List_Item_Tooltip_Resize_Cb list_item_tooltip_resize[2];
   struct {
        Ecore_Event_Handler *iq;
        Ecore_Event_Handler *presence;
        Ecore_Event_Handler *message;
   } event_handlers;
   Shotgun_Auth *account; /* user's account */
};

typedef struct
{
   Contact_List *cl;
   Evas_Object *win;
   Evas_Object *box;
   Evas_Object *toolbar;
   Evas_Object *pager;
   Eina_List *contacts;
} Chat_Window;

struct Contact
{
   Shotgun_User *base;
   Shotgun_User_Info *info;
   Shotgun_Event_Presence *cur; /* the current presence; should NOT be in plist */
   Eina_List *plist; /* list of presences with lower priority than cur */

   /* the next 3 are just convenience pointers to the user's current X */
   Shotgun_User_Status status; /* user's current status */
   int priority; /* user's current priority */
   const char *description; /* user's current status message */

   const char *force_resource; /* always send to this resource if set */
   const char *last_conv; /* entire conversation with user to keep conversations fluid when windows are opened/closed */
   const char *tooltip_label; /* label for contact list item tooltip */
   Ecore_Timer *tooltip_timer; /* timer for setting tooltip_changed */
   void *list_item; /* the grid/list item object */
   Chat_Window *chat_window; /* the chat window for the contact (if open) */
   Evas_Object *chat_panes; /* box in the chat window belonging to contact */
   Elm_Object_Item *chat_tb_item; /* contact's toolbar item */
   Evas_Object *chat_buffer; /* chat buffer of the conversation */
   Evas_Object *chat_input; /* input entry for the conversation */
   Evas_Object *chat_jid_menu; /* menu object for the submenu in the chat window */
   Evas_Object *status_line; /* status entry inside the frame at top */
   Evas_Object *animated; /* animated object for status */
   Ecore_Animator *animator; /* animator for status */
   Contact_List *list; /* the owner list */
   FILE *log; /* log of contact */
   const char *logdir; /* directory where contact's logs are stored */
   Eina_Bool tooltip_changed : 1; /* when set, tooltip_label will be re-created */
   Eina_Bool ignore_resource : 1; /* when set, priority will be ignored and messages will be sent to all resources */
   Eina_Bool logdir_exists : 1; /* true only if a contact has an existing log directory */
};

typedef struct
{
   Ecore_Con_Url *url;
   Eina_Binbuf *buf;
   const char *addr;
   unsigned long long timestamp;
   Contact_List *cl;
} Image;

Contact_List *contact_list_new(Shotgun_Auth *auth, Shotgun_Settings *ss);
void contact_list_user_add(Contact_List *cl, Contact *c);
void contact_list_user_del(Contact *c, Shotgun_Event_Presence *ev);

void chat_window_new(Contact_List *cl);
void chat_window_chat_new(Contact *c, Chat_Window *cw, Eina_Bool focus);
void chat_window_free(Chat_Window *cw, Evas_Object *obj __UNUSED__, const char *ev __UNUSED__);
void chat_message_status(Contact *c, Shotgun_Event_Message *msg);
void chat_message_insert(Contact *c, const char *from, const char *msg, Eina_Bool me);

void chat_image_add(Contact_List *cl, const char *url);
void chat_image_free(Image *i);
void chat_image_cleanup(Contact_List *cl);
void chat_conv_image_show(Contact *c, Evas_Object *obj, Elm_Entry_Anchor_Info *ev);
void chat_conv_image_hide(Contact *c, Evas_Object *obj, Elm_Entry_Anchor_Info *ev);
Eina_Bool chat_image_data(void *d __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Url_Data *ev);
Eina_Bool chat_image_complete(void *d __UNUSED__, int type __UNUSED__, Ecore_Con_Event_Url_Complete *ev);

Shotgun_Event_Presence *contact_presence_get(Contact *c);
void contact_presence_set(Contact *c, Shotgun_Event_Presence *cur);
void contact_presence_clear(Contact *c);
void contact_resource_set(Contact *c, Evas_Object *obj __UNUSED__, Elm_Object_Item *ev);
void contact_resource_menu_setup(Contact *c, Evas_Object *menu);
const char *contact_name_get(Contact *c);
void contact_jids_menu_del(Contact *c, const char *jid);
void contact_free(Contact *c);
Contact *do_something_with_user(Contact_List *cl, Shotgun_User *user);
Eina_Bool contact_chat_window_current(Contact *c);
void contact_chat_window_animator_add(Contact *c);
void contact_chat_window_animator_del(Contact *c);
void contact_chat_window_close(Contact *c);
void contact_subscription_set(Contact *c, Shotgun_Presence_Type type, Shotgun_User_Subscription sub);

Eina_Bool ui_eet_init(Shotgun_Auth *auth);
void ui_eet_dummy_add(const char *url);
Eina_Bool ui_eet_dummy_check(const char *url);
int ui_eet_image_add(const char *url, Eina_Binbuf *buf, unsigned long long timestamp);
void ui_eet_image_del(const char *url);
Eina_Binbuf *ui_eet_image_get(const char *url, unsigned long long timestamp);
void ui_eet_image_ping(const char *url, unsigned long long timestamp);
void ui_eet_shutdown(Shotgun_Auth *auth);
Shotgun_Auth *ui_eet_auth_get(const char *name, const char *domain);
void ui_eet_auth_set(Shotgun_Auth *auth, Shotgun_Settings *settings, Eina_Bool use_auth);
Eina_Bool ui_eet_userinfo_add(Shotgun_Auth *auth, Evas_Object *img, Shotgun_User_Info *info);
Shotgun_User_Info *ui_eet_userinfo_get(Shotgun_Auth *auth, const char *jid);
Shotgun_Settings *ui_eet_settings_get(Shotgun_Auth *auth);
void ui_eet_settings_set(Shotgun_Auth *auth, Shotgun_Settings *ss);
Eina_Bool ui_eet_idler_start(Contact_List *cl);
void ui_eet_presence_set(Shotgun_Auth *auth);
Eina_Bool ui_eet_presence_get(Shotgun_Auth *auth);

#ifdef HAVE_DBUS
void ui_dbus_signal_message_self(Contact_List *cl, const char *jid, const char *s);
void ui_dbus_signal_message(Contact_List *cl, Contact *c, Shotgun_Event_Message *msg);
void ui_dbus_signal_status_self(Contact_List *cl);
void ui_dbus_init(Contact_List *cl);
# ifdef HAVE_NOTIFY
void ui_dbus_notify(Contact_List *cl, Evas_Object *img, const char *from, const char *msg);
# endif
#endif

#ifdef HAVE_AZY
void ui_azy_init(Contact_List *cl);
Eina_Bool ui_azy_connect(Contact_List *cl);
void ui_azy_shutdown(Contact_List *cl);
#endif

Eina_Bool event_iq_cb(Contact_List *cl, int type __UNUSED__, Shotgun_Event_Iq *ev);
Eina_Bool event_presence_cb(Contact_List *cl, int type __UNUSED__, Shotgun_Event_Presence *ev);
Eina_Bool event_message_cb(Contact_List *cl, int type __UNUSED__, Shotgun_Event_Message *msg);

const char *logging_dir_get(void);
Eina_Bool logging_dir_create(Contact_List *cl);
Eina_Bool logging_contact_init(Contact *c);
Eina_Bool logging_contact_file_refresh(Contact *c);
void logging_contact_file_close(Contact *c);

void settings_new(Contact_List *cl);
void settings_toggle(Contact_List *cl, Evas_Object *obj, void *event_info);

Eina_Bool util_userinfo_eq(Shotgun_User_Info *a, Shotgun_User_Info *b);
const char *util_configdir_get(void);
Eina_Bool util_configdir_create(void);

#endif /* __UI_H */
