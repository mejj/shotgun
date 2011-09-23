#include "ui.h"

Eina_Bool
event_iq_cb(Contact_List *cl, int type __UNUSED__, Shotgun_Event_Iq *ev)
{
   Contact *c;
   switch(ev->type)
     {
      case SHOTGUN_IQ_EVENT_TYPE_ROSTER:
        {
           Shotgun_User *user;
           EINA_LIST_FREE(ev->ev, user)
             {
                c = do_something_with_user(cl, user);
                contact_subscription_set(c, 0, user->subscription);
                if (user->subscription == SHOTGUN_USER_SUBSCRIPTION_REMOVE)
                  {
                     Shotgun_Event_Presence *pres;

                     EINA_LIST_FREE(c->plist, pres)
                       shotgun_event_presence_free(pres);
                     contact_list_user_del(c, c->cur);
                     if (c->base == user) c->base = NULL;
                     shotgun_user_free(user);
                     eina_hash_del_by_data(cl->users, c);
                     cl->users_list = eina_list_remove(cl->users_list, c);
                     contact_free(c);
                     continue;
                  }
                if (c->list_item)
                  {
                     if ((!cl->view) && (!c->status) && (c->base->subscription == SHOTGUN_USER_SUBSCRIPTION_BOTH))
                       contact_list_user_del(c, NULL);
                     else
                       cl->list_item_update[cl->mode](c->list_item);
                  }
                else
                  {
                     if (cl->view || (user->subscription != SHOTGUN_USER_SUBSCRIPTION_BOTH) || user->subscription_pending)
                       contact_list_user_add(cl, c);
                  }
             }
           break;
        }
      case SHOTGUN_IQ_EVENT_TYPE_INFO:
        {
           Shotgun_User_Info *info = ev->ev;

           c = eina_hash_find(cl->users, info->jid);
           if (!c)
             {
                ERR("WTF!");
                break;
             }
           shotgun_user_info_free(c->info);
           if (c->cur && c->cur->photo)
             {
                INF("Found contact photo sha1: %s", c->cur->photo);
                info->photo.sha1 = eina_stringshare_ref(c->cur->photo);
             }
           c->info = info;
           ev->ev = NULL;
           if (c->list_item && (info->photo.data || info->full_name)) cl->list_item_update[cl->mode](c->list_item);

           ui_eet_userinfo_add(cl->account, info);
           break;
        }
      default:
        ERR("WTF!");
     }
   return EINA_TRUE;
}

static int
_list_sort_cb(Shotgun_Event_Presence *a, Shotgun_Event_Presence *b)
{
   return a->priority - b->priority;
}

Eina_Bool
event_presence_cb(Contact_List *cl, int type __UNUSED__, Shotgun_Event_Presence *ev)
{
   Contact *c;
   Shotgun_Event_Presence *pres;
   char *jid, *p;
   Eina_List *l;

   p = strchr(ev->jid, '/');
   if (p) jid = strndupa(ev->jid, p - ev->jid);
   else jid = (char*)ev->jid;
   c = eina_hash_find(cl->users, jid);
   if (!c) return EINA_TRUE;

   if (!ev->status)
     {
        contact_list_user_del(c, ev);
        return EINA_TRUE;
     }
   if (c->cur)
     {
        switch (ev->type)
          {
           case SHOTGUN_PRESENCE_TYPE_SUBSCRIBE:
           case SHOTGUN_PRESENCE_TYPE_UNSUBSCRIBE:
             contact_subscription_set(c, ev->type, c->base->subscription);
             if (c->list_item)
               cl->list_item_update[cl->mode](c->list_item);
             else
               contact_list_user_add(cl, c);
             return ECORE_CALLBACK_RENEW;
           default:
             break;
          }
     }
   /* if current resource is not event resource */
   if ((!c->cur) || (ev->jid != c->cur->jid))
     {
        EINA_LIST_FOREACH(c->plist, l, pres)
          {
             /* update existing resource if found */
             if (ev->jid != pres->jid) continue;

             pres->priority = ev->priority;
             pres->status = ev->status;

             eina_stringshare_replace(&pres->description, NULL);
             if (ev->description && ev->description[0])
               pres->description = eina_stringshare_ref(ev->description);

             eina_stringshare_replace(&pres->photo, NULL);
             if (ev->photo && ev->photo[0])
               pres->photo = eina_stringshare_ref(ev->photo);

             pres->vcard = ev->vcard;
             break;
          }
        /* if not found, copy */
        if ((!pres) || (pres->jid != ev->jid))
          {
             pres = calloc(1, sizeof(Shotgun_Event_Presence));
             pres->jid = eina_stringshare_ref(ev->jid);
             pres->priority = ev->priority;
             pres->status = ev->status;
             pres->description = eina_stringshare_ref(ev->description);
             pres->photo = eina_stringshare_ref(ev->photo);
             pres->vcard = ev->vcard;
          }
        /* if found, update */
        else if (pres && (pres->jid == ev->jid))
          {
             pres->priority = ev->priority;
             pres->status = ev->status;
             if (pres->description != ev->description)
               {
                  eina_stringshare_del(pres->description);
                  pres->description = eina_stringshare_ref(ev->description);
               }
             if (pres->photo != ev->photo)
               {
                  eina_stringshare_del(pres->photo);
                  pres->photo = eina_stringshare_ref(ev->photo);
               }             pres->vcard = ev->vcard;
             /* must sort! */
             c->plist = eina_list_sort(c->plist, 0, (Eina_Compare_Cb)_list_sort_cb);
          }
        /* if not the current resource, update current */
        if (c->cur)
          {
             /* if current resource has no photo, use low priority photo */
             if (pres->photo && (!c->cur->photo))
               c->cur->photo = eina_stringshare_ref(pres->photo);
             c->cur->vcard |= pres->vcard;
             /* if lower priority, add to plist */
             if (ev->priority < c->cur->priority)
               {
                  if ((!l) || (l->data != pres))
                    c->plist = eina_list_sorted_insert(c->plist, (Eina_Compare_Cb)_list_sort_cb, pres);
                  /* if vcard available and (not retrieved || not most recent) */
                  if (ev->vcard && ((!c->info) || (c->cur && c->info &&
                      ((c->info->photo.sha1 != c->cur->photo) ||
                       (c->cur->photo && (!c->info->photo.data))))))
                    shotgun_iq_vcard_get(ev->account, c->base->jid);
                  return EINA_TRUE;
               }
             c->plist = eina_list_remove(c->plist, pres);
             c->plist = eina_list_sorted_insert(c->plist, (Eina_Compare_Cb)_list_sort_cb, c->cur);
          }
        c->cur = pres;
     }

   c->status = c->cur->status;
   /* if status description exists and isn't the same as current status description */
   if (c->status_line && (c->description != c->cur->description))
     {
        elm_entry_entry_set(c->status_line, "");
        if (c->cur->description) elm_entry_entry_append(c->status_line, c->cur->description);
     }
   c->description = c->cur->description;
   c->priority = c->cur->priority;
   /* if offline view or contact has a subscription, create/update list item */
   if (cl->view || (c->base->subscription > SHOTGUN_USER_SUBSCRIPTION_NONE))
     {
        c->tooltip_changed = EINA_TRUE;
        /* if no list item, create */
        if (!c->list_item)
          {
             contact_list_user_add(cl, c);
             /* if vcard available, fetch */
             if (ev->vcard)
               {
                  c->info = ui_eet_userinfo_get(cl->account, c->base->jid);
                  if (c->info) cl->list_item_update[cl->mode](c->list_item);
                  if ((!c->info) || (c->cur && c->info &&
                      ((c->info->photo.sha1 != c->cur->photo) || (c->cur->photo && (!c->info->photo.data)))))
                    shotgun_iq_vcard_get(ev->account, c->base->jid);
               }
          }
        /* otherwise, update */
        else
          {
             /* if vcard available and (not retrieved || not most recent) */
             if (ev->vcard && ((!c->info) || (c->cur && c->info &&
                 ((c->info->photo.sha1 != c->cur->photo) || (c->cur->photo && (!c->info->photo.data))))))
               shotgun_iq_vcard_get(ev->account, c->base->jid);
             cl->list_item_update[cl->mode](c->list_item);
          }
     }
   return EINA_TRUE;
}

Eina_Bool
event_message_cb(Contact_List *cl, int type __UNUSED__, Shotgun_Event_Message *msg)
{
   Contact *c;
   char *jid, *p;

   jid = strdupa(msg->jid);
   p = strchr(jid, '/');
   if (p) *p = 0;
   c = eina_hash_find(cl->users, jid);
   if (!c) return EINA_TRUE;

   if (!c->chat_window)
     {
        if (msg->msg /*|| (msg->status && cl->open window on status)*/)
          chat_window_new(c);
     }

   if (msg->msg)
     {
        chat_message_insert(c, contact_name_get(c), msg->msg, EINA_FALSE);
#ifdef HAVE_DBUS
        ui_dbus_signal_message(cl, c, msg);
#endif
     }
   if (c->chat_window && msg->status)
     chat_message_status(c, msg);

   return EINA_TRUE;
}
