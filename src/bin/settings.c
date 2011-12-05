#include "ui.h"

#define SETTINGS_FRAME(LABEL) do { \
   fr = elm_frame_add(ui->win); \
   EXPAND(fr); \
   FILL(fr); \
   elm_object_text_set(fr, LABEL); \
   elm_box_pack_end(box, fr); \
   evas_object_show(fr); \
\
   frbox = elm_box_add(ui->win); \
   EXPAND(frbox); \
   FILL(frbox); \
   elm_object_content_set(fr, frbox); \
   evas_object_show(frbox); \
} while (0)

#define SETTINGS_CHECK(LABEL, POINTER, TOOLTIP) do { \
   ck = elm_check_add(ui->win); \
   EXPAND(ck); \
   FILL(ck); \
   elm_object_text_set(ck, LABEL); \
   elm_check_state_pointer_set(ck, &ui->settings->POINTER); \
   elm_object_tooltip_text_set(ck, TOOLTIP); \
   elm_tooltip_window_mode_set(ck, EINA_TRUE); \
   elm_box_pack_end(frbox, ck); \
   evas_object_show(ck); \
} while (0)

#define SETTINGS_SLIDER(LABEL, TOOLTIP, UNITS, MAX, CB) do { \
   sl = elm_slider_add(ui->win); \
   EXPAND(sl); \
   FILL(sl); \
   elm_slider_unit_format_set(sl, UNITS); \
   elm_slider_min_max_set(sl, 0, MAX); \
   elm_object_text_set(sl, LABEL); \
   elm_object_tooltip_text_set(sl, TOOLTIP); \
   elm_tooltip_window_mode_set(sl, EINA_TRUE); \
   evas_object_smart_callback_add(sl, "delay,changed", (Evas_Smart_Cb)_settings_ ##CB## _change, ui); \
   elm_box_pack_end(frbox, sl); \
   evas_object_show(sl); \
} while (0)

static void
_settings_image_size_change(UI_WIN *ui, Evas_Object *obj, void *event_info __UNUSED__)
{
   ui->settings->allowed_image_size = elm_slider_value_get(obj);
}

static void
_settings_image_age_change(UI_WIN *ui, Evas_Object *obj, void *event_info __UNUSED__)
{
   ui->settings->allowed_image_age = elm_slider_value_get(obj);
}

static void
_settings_logging_change(Contact_List *cl, Evas_Object *obj, void *event_info __UNUSED__)
{
   Eina_List *l, *ll;
   const char *dir;
   Chat_Window *cw;
   Contact *c;

   if (!elm_check_state_get(obj))
     {
        /* close existing logs */
        EINA_LIST_FOREACH(cl->users_list, l, c)
          logging_contact_file_close(c);
        return;
     }

   dir = logging_dir_get();
   if (!dir[0]) logging_dir_create(cl);
   EINA_LIST_FOREACH(cl->chat_wins, l, cw)
     {
        EINA_LIST_FOREACH(cw->contacts, ll, c)
          /* open logs for all open chats */
          logging_contact_file_refresh(c);
     }
}

static void
_settings_chat_typing_change(Contact_List *cl, Evas_Object *obj, void *event_info __UNUSED__)
{
   Eina_List *l, *ll;
   Chat_Window *cw;
   Contact *c;

   if (!elm_check_state_get(obj))
     {
        EINA_LIST_FOREACH(cl->chat_wins, l, cw)
          {
             EINA_LIST_FOREACH(cw->contacts, ll, c)
               evas_object_smart_callback_del(c->chat_input, "changed,user", (Evas_Smart_Cb)contact_chat_window_typing);
          }
        return;
     }
   EINA_LIST_FOREACH(cl->chat_wins, l, cw)
     {
        EINA_LIST_FOREACH(cw->contacts, ll, c)
          evas_object_smart_callback_add(c->chat_input, "changed,user", (Evas_Smart_Cb)contact_chat_window_typing, c);
     }
}

void
settings_new(UI_WIN *ui)
{
   Evas_Object *scr, *ic, *back, *box, *ck, *fr, *frbox, *sl;
   int init;
   Contact_List *cl;
   Login_Window *lw;

   IF_UI_IS_LOGIN(ui) lw = (Login_Window*)ui;
   else cl = (Contact_List*)ui;

   ui->settings_box = box = elm_box_add(ui->win);
   EXPAND(box);
   FILL(box);
   elm_flip_content_back_set(ui->flip, box);
   evas_object_show(box);

   ic = elm_icon_add(ui->win);
   elm_icon_standard_set(ic, "back");
   evas_object_show(ic);
   back = elm_button_add(ui->win);
   elm_object_content_set(back, ic);
   IF_UI_IS_LOGIN(ui)
     elm_object_tooltip_text_set(back, "Return to login");
   else
     elm_object_tooltip_text_set(back, "Return to contact list");
   elm_tooltip_window_mode_set(back, EINA_TRUE);
   WEIGHT(back, 0, 0);
   ALIGN(back, 0, 0);
   elm_box_pack_end(box, back);
   evas_object_smart_callback_add(back, "clicked", (Evas_Smart_Cb)settings_toggle, ui);
   evas_object_show(back);

   scr = elm_scroller_add(ui->win);
   EXPAND(scr);
   FILL(scr);
   elm_box_pack_end(box, scr);

   box = elm_box_add(ui->win);
   EXPAND(box);
   FILL(box);
   evas_object_show(box);

   elm_object_content_set(scr, box);
   elm_scroller_bounce_set(scr, EINA_FALSE, EINA_FALSE);
   elm_scroller_policy_set(scr, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   evas_object_show(scr);

   SETTINGS_FRAME("Account");
   SETTINGS_CHECK("Save account info", enable_account_info, "Remember account name and password");
   IF_UI_IS_LOGIN(ui) elm_object_disabled_set(ck, EINA_TRUE);
   SETTINGS_CHECK("Remember last account", enable_last_account, "Automatically sign in with current account on next run");
   IF_UI_IS_LOGIN(ui) elm_object_disabled_set(ck, EINA_TRUE);
   SETTINGS_CHECK("Remember last presence", enable_presence_save, "Automatically set last-used presence on next sign in");
   IF_UI_IS_LOGIN(ui) elm_object_disabled_set(ck, EINA_TRUE);

   SETTINGS_FRAME("Application");
   SETTINGS_CHECK("Enable single window mode", enable_illume, "Use a single window for the application - REQUIRES RESTART (embedded friendly)");
   SETTINGS_CHECK("Disable automatic reconnect", disable_reconnect, "Disable automatic reconnection when disconnected");

#ifdef HAVE_NOTIFY
   SETTINGS_FRAME("DBus");
   SETTINGS_CHECK("Disable notifications", disable_notify, "Disables use of notification popups");
#endif

   init = ecore_con_url_init();
   if (init)
     SETTINGS_FRAME("Images");
   else
     SETTINGS_FRAME("Images (DISABLED: REQUIRES CURL SUPPORT IN ECORE)");
   SETTINGS_CHECK("Disable automatic image fetching", disable_image_fetch, "Disables background fetching of images");
   if (init)
     {
        SETTINGS_SLIDER("Max image age", "Number of days to save linked images on disk before deleting them",
                        "%1.0f days", 60, image_age);
        SETTINGS_SLIDER("Max image memory", "Total size of images to keep in memory",
                        "%1.0f MB", 512, image_size);
     }
   else
     {
        elm_check_state_set(ck, EINA_TRUE);
        elm_object_disabled_set(ck, EINA_TRUE);
     }

   SETTINGS_FRAME("Messages");
   SETTINGS_CHECK("Send keyboard events", enable_chat_typing, "Send additional notifications to contacts when you start or stop typing to them");
   IF_UI_IS_NOT_LOGIN(ui) evas_object_smart_callback_add(ck, "changed", (Evas_Smart_Cb)_settings_chat_typing_change, cl);
   SETTINGS_CHECK("Focus chat window on message", enable_chat_focus, "Focus chat window whenever message is received");
   SETTINGS_CHECK("Promote contact on message", enable_chat_promote, "Move contact to top of list when message is received");
   SETTINGS_CHECK("Always select new chat tabs", enable_chat_newselect, "When a message is received which would open a new tab, make that tab active");
   SETTINGS_CHECK("Log messages to disk", enable_logging, "All messages sent or received will appear in ~/.config/shotgun/logs");
   IF_UI_IS_LOGIN(ui) elm_object_disabled_set(ck, EINA_TRUE);
   IF_UI_IS_NOT_LOGIN(ui) evas_object_smart_callback_add(ck, "changed", (Evas_Smart_Cb)_settings_logging_change, cl);
}

void
settings_toggle(UI_WIN *ui, Evas_Object *obj __UNUSED__, void *event_info)
{
   Contact_List *cl;
   Login_Window *lw;

   IF_UI_IS_LOGIN(ui) lw = (Login_Window*)ui;
   else cl = (Contact_List*)ui;

   IF_UI_IS_NOT_LOGIN(ui)
     {
        if ((!cl->image_cleaner) && cl->settings->allowed_image_age)
          ui_eet_idler_start(cl);
        chat_image_cleanup(cl);
     }
   IF_ILLUME(ui)
     {
        if (elm_flip_front_get(ui->flip))
          elm_object_text_set(ui->illume_frame, "Settings");
        else
          elm_object_text_set(ui->illume_frame, "Contacts");
     }
   IF_UI_IS_NOT_LOGIN(ui)
     {
        if (event_info) elm_toolbar_item_selected_set(event_info, EINA_FALSE);
     }
   elm_flip_go(ui->flip, ELM_FLIP_ROTATE_Y_CENTER_AXIS);
}

void
settings_finagle(UI_WIN *ui)
{
   int argc, x;
   char **argv;
   Shotgun_Settings *ss;

   ss = ui_eet_settings_get(ui->account);
   ecore_app_args_get(&argc, &argv);
   if (ss) ui->settings = ss;
   else
     {
        int x, dash = 0;

        ss = ui->settings = calloc(1, sizeof(Shotgun_Settings));
        /* don't count --enable/disable args */
        for (x = 1; x < argc; x++)
          if (argv[x][0] == '-') dash++;
        switch (argc - dash)
          {
           case 1:
             ui->settings->enable_last_account = EINA_TRUE;
           case 3:
             ui->settings->enable_account_info = EINA_TRUE;
           default:
             break;
          }
        shotgun_settings_set(ui->account, ss);
     }

   ss->ui = ui;
   for (x = 1; x < argc; x++)
     {
        if ((!strcmp(argv[x], "--illume")) || (!strcmp(argv[x], "--enable-illume")))
          ui->settings->enable_illume = EINA_TRUE;
        if (!strcmp(argv[x], "--disable-illume"))
          ui->settings->enable_illume = EINA_FALSE;
     }
   INF("ILLUME: %s", ui->settings->enable_illume ? "ENABLED" : "DISABLED");
}
