/**
 * xrdp: A Remote Desktop Protocol server.
 *
 * Copyright (C) Jay Sorg 2012-2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include "libxrdp.h"
#include "xrdp_rail.h"

/* [MS-RDPERP]: Remote Desktop Protocol:
   Remote Programs Virtual Channel Extension
   http://msdn.microsoft.com/en-us/library/cc242568(v=prot.10) */

/*****************************************************************************/
/* RAIL */
/* returns error */
int
xrdp_orders_send_window_delete(struct xrdp_orders *self, int window_id)
{
    int order_size;
    int order_flags;
    int field_present_flags;

    order_size = 11;
    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    field_present_flags = WINDOW_ORDER_TYPE_WINDOW | WINDOW_ORDER_STATE_DELETED;
    out_uint32_le(self->out_s, field_present_flags);
    /* windowId (4 bytes) */
    out_uint32_le(self->out_s, window_id);
    return 0;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
/* flags can contain WINDOW_ORDER_STATE_NEW and/or
   WINDOW_ORDER_FIELD_ICON_BIG */
int
xrdp_orders_send_window_cached_icon(struct xrdp_orders *self,
                                    int window_id, int cache_entry,
                                    int cache_id, int flags)
{
    int order_size;
    int order_flags;
    int field_present_flags;

    order_size = 14;
    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    field_present_flags = flags | WINDOW_ORDER_TYPE_WINDOW |
                          WINDOW_ORDER_CACHED_ICON;
    out_uint32_le(self->out_s, field_present_flags);
    /* windowId (4 bytes) */
    out_uint32_le(self->out_s, window_id);
    /* CacheEntry (2 bytes) */
    out_uint16_le(self->out_s, cache_entry);
    /* CacheId (1 byte) */
    out_uint8(self->out_s, cache_id);
    return 0;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
static int
xrdp_orders_send_ts_icon(struct stream *s, int cache_entry, int cache_id,
                         struct rail_icon_info *icon_info)
{
    int use_cmap;

    use_cmap = 0;

    if ((icon_info->bpp == 1) || (icon_info->bpp == 2) || (icon_info->bpp == 4))
    {
        use_cmap = 1;
    }

    /* TS_ICON_INFO */
    out_uint16_le(s, cache_entry);
    out_uint8(s, cache_id);
    out_uint8(s, icon_info->bpp);
    out_uint16_le(s, icon_info->width);
    out_uint16_le(s, icon_info->height);

    if (use_cmap)
    {
        out_uint16_le(s, icon_info->cmap_bytes);
    }

    out_uint16_le(s, icon_info->mask_bytes);
    out_uint16_le(s, icon_info->data_bytes);
    out_uint8p(s, icon_info->mask, icon_info->mask_bytes);

    if (use_cmap)
    {
        out_uint8p(s, icon_info->cmap, icon_info->cmap_bytes);
    }

    out_uint8p(s, icon_info->data, icon_info->data_bytes);
    return 0;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
/* flags can contain WINDOW_ORDER_STATE_NEW and/or
   WINDOW_ORDER_FIELD_ICON_BIG */
int
xrdp_orders_send_window_icon(struct xrdp_orders *self,
                             int window_id, int cache_entry, int cache_id,
                             struct rail_icon_info *icon_info,
                             int flags)
{
    int order_size;
    int order_flags;
    int field_present_flags;
    int use_cmap;

    use_cmap = 0;

    if ((icon_info->bpp == 1) || (icon_info->bpp == 2) || (icon_info->bpp == 4))
    {
        use_cmap = 1;
    }

    order_size = 23 + icon_info->mask_bytes + icon_info->data_bytes;

    if (use_cmap)
    {
        order_size += icon_info->cmap_bytes + 2;
    }

    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    field_present_flags = flags | WINDOW_ORDER_TYPE_WINDOW |
                          WINDOW_ORDER_ICON;
    out_uint32_le(self->out_s, field_present_flags);
    /* windowId (4 bytes) */
    out_uint32_le(self->out_s, window_id);

    xrdp_orders_send_ts_icon(self->out_s, cache_entry, cache_id, icon_info);

    return 0;
}

/*****************************************************************************/
/* returns error */
static int
xrdp_orders_send_as_unicode(struct stream *s, const char *text)
{
    int str_chars;
    int index;
    int i32;
    int len;
    twchar *wdst;

    len = g_strlen(text) + 1;

    wdst = (twchar *) g_malloc(sizeof(twchar) * len, 1);
    if (wdst == 0)
    {
        return 1;
    }
    str_chars = g_mbstowcs(wdst, text, len);
    if (str_chars > 0)
    {
        i32 = str_chars * 2;
        out_uint16_le(s, i32);
        for (index = 0; index < str_chars; index++)
        {
            i32 = wdst[index];
            out_uint16_le(s, i32);
        }
    }
    else
    {
        out_uint16_le(s, 0);
    }
    g_free(wdst);
    return 0;
}

/*****************************************************************************/
static int
xrdp_orders_get_unicode_bytes(const char *text)
{
    int num_chars;

    num_chars = g_mbstowcs(0, text, 0);
    if (num_chars < 0)
    {
        /* g_mbstowcs failed, we ignore that text by returning zero bytes */
        num_chars = 0;
    }
    else
    {
        /* calculate the number of bytes of the resulting null-terminated wide-string */
        num_chars = (num_chars + 1) * 2;
    }

    return num_chars;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
/* flags can contain WINDOW_ORDER_STATE_NEW */
int
xrdp_orders_send_window_new_update(struct xrdp_orders *self, int window_id,
                                   struct rail_window_state_order *window_state,
                                   int flags)
{
    int order_size;
    int order_flags;
    int field_present_flags;
    int index;

    order_size = 11;
    field_present_flags = flags | WINDOW_ORDER_TYPE_WINDOW;

    if (field_present_flags & WINDOW_ORDER_FIELD_OWNER)
    {
        /* ownerWindowId (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_STYLE)
    {
        /* style (4 bytes) */
        order_size += 4;
        /* extendedStyle (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_SHOW)
    {
        /* showState (1 byte) */
        order_size += 1;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_TITLE)
    {
        /* titleInfo */
        order_size += xrdp_orders_get_unicode_bytes(window_state->title_info);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_CLIENT_AREA_OFFSET)
    {
        /* clientOffsetX (4 bytes) */
        order_size += 4;
        /* clientOffsetY (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_CLIENT_AREA_SIZE)
    {
        /* clientAreaWidth (4 bytes) */
        order_size += 4;
        /* clientAreaHeight (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_RP_CONTENT)
    {
        /* RPContent (1 byte) */
        order_size += 1;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_ROOT_PARENT)
    {
        /* rootParentHandle (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_OFFSET)
    {
        /* windowOffsetX (4 bytes) */
        order_size += 4;
        /* windowOffsetY (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_CLIENT_DELTA)
    {
        /* windowClientDeltaX (4 bytes) */
        order_size += 4;
        /* windowClientDeltaY (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_SIZE)
    {
        /* windowWidth (4 bytes) */
        order_size += 4;
        /* windowHeight (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_RECTS)
    {
        /* numWindowRects (2 bytes) */
        order_size += 2;
        order_size += 8 * window_state->num_window_rects;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_VIS_OFFSET)
    {
        /* visibleOffsetX (4 bytes) */
        order_size += 4;
        /* visibleOffsetY (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_VISIBILITY)
    {
        /* numVisibilityRects (2 bytes) */
        order_size += 2;
        order_size += 8 * window_state->num_visibility_rects;
    }

    if (order_size < 12)
    {
        /* no flags set */
        return 0;
    }

    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    out_uint32_le(self->out_s, field_present_flags);
    /* windowId (4 bytes) */
    out_uint32_le(self->out_s, window_id);

    if (field_present_flags & WINDOW_ORDER_FIELD_OWNER)
    {
        /* ownerWindowId (4 bytes) */
        out_uint32_le(self->out_s, window_state->owner_window_id);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_STYLE)
    {
        /* style (4 bytes) */
        out_uint32_le(self->out_s, window_state->style);
        /* extendedStyle (4 bytes) */
        out_uint32_le(self->out_s, window_state->extended_style);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_SHOW)
    {
        /* showState (1 byte) */
        out_uint8(self->out_s, window_state->show_state);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_TITLE)
    {
        /* titleInfo */
        xrdp_orders_send_as_unicode(self->out_s, window_state->title_info);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_CLIENT_AREA_OFFSET)
    {
        /* clientOffsetX (4 bytes) */
        out_uint32_le(self->out_s, window_state->client_offset_x);
        /* clientOffsetY (4 bytes) */
        out_uint32_le(self->out_s, window_state->client_offset_y);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_CLIENT_AREA_SIZE)
    {
        /* clientAreaWidth (4 bytes) */
        out_uint32_le(self->out_s, window_state->client_area_width);
        /* clientAreaHeight (4 bytes) */
        out_uint32_le(self->out_s, window_state->client_area_height);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_RP_CONTENT)
    {
        /* RPContent (1 byte) */
        out_uint8(self->out_s, window_state->rp_content);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_ROOT_PARENT)
    {
        /* rootParentHandle (4 bytes) */
        out_uint32_le(self->out_s, window_state->root_parent_handle);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_OFFSET)
    {
        /* windowOffsetX (4 bytes) */
        out_uint32_le(self->out_s, window_state->window_offset_x);
        /* windowOffsetY (4 bytes) */
        out_uint32_le(self->out_s, window_state->window_offset_y);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_CLIENT_DELTA)
    {
        /* windowClientDeltaX (4 bytes) */
        out_uint32_le(self->out_s, window_state->window_client_delta_x);
        /* windowClientDeltaY (4 bytes) */
        out_uint32_le(self->out_s, window_state->window_client_delta_y);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_SIZE)
    {
        /* windowWidth (4 bytes) */
        out_uint32_le(self->out_s, window_state->window_width);
        /* windowHeight (4 bytes) */
        out_uint32_le(self->out_s, window_state->window_height);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_WND_RECTS)
    {
        /* numWindowRects (2 bytes) */
        out_uint16_le(self->out_s, window_state->num_window_rects);

        for (index = 0; index < window_state->num_window_rects; index++)
        {
            out_uint16_le(self->out_s, window_state->window_rects[index].left);
            out_uint16_le(self->out_s, window_state->window_rects[index].top);
            out_uint16_le(self->out_s, window_state->window_rects[index].right);
            out_uint16_le(self->out_s, window_state->window_rects[index].bottom);
        }
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_VIS_OFFSET)
    {
        /* visibleOffsetX (4 bytes) */
        out_uint32_le(self->out_s, window_state->visible_offset_x);
        /* visibleOffsetY (4 bytes) */
        out_uint32_le(self->out_s, window_state->visible_offset_y);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_VISIBILITY)
    {
        /* numVisibilityRects (2 bytes) */
        out_uint16_le(self->out_s, window_state->num_visibility_rects);

        for (index = 0; index < window_state->num_visibility_rects; index++)
        {
            out_uint16_le(self->out_s, window_state->visibility_rects[index].left);
            out_uint16_le(self->out_s, window_state->visibility_rects[index].top);
            out_uint16_le(self->out_s, window_state->visibility_rects[index].right);
            out_uint16_le(self->out_s, window_state->visibility_rects[index].bottom);
        }
    }

    return 0;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
int
xrdp_orders_send_notify_delete(struct xrdp_orders *self, int window_id,
                               int notify_id)
{
    int order_size;
    int order_flags;
    int field_present_flags;

    order_size = 15;
    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    field_present_flags = WINDOW_ORDER_TYPE_NOTIFY | WINDOW_ORDER_STATE_DELETED;
    out_uint32_le(self->out_s, field_present_flags);
    /* windowId (4 bytes) */
    out_uint32_le(self->out_s, window_id);
    /* notifyIconId (4 bytes) */
    out_uint32_le(self->out_s, notify_id);
    return 0;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
/* flags can contain WINDOW_ORDER_STATE_NEW */
int
xrdp_orders_send_notify_new_update(struct xrdp_orders *self,
                                   int window_id, int notify_id,
                                   struct rail_notify_state_order *notify_state,
                                   int flags)
{
    int order_size;
    int order_flags;
    int field_present_flags;
    int use_cmap;

    order_size = 15;
    field_present_flags = flags | WINDOW_ORDER_TYPE_NOTIFY;

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_VERSION)
    {
        /* Version (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_TIP)
    {
        /* ToolTip (variable) UNICODE_STRING */
        order_size += xrdp_orders_get_unicode_bytes(notify_state->tool_tip);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_INFO_TIP)
    {
        /* InfoTip (variable) TS_NOTIFY_ICON_INFOTIP */
        /* UNICODE_STRING */
        order_size += xrdp_orders_get_unicode_bytes(notify_state->infotip.title);
        /* UNICODE_STRING */
        order_size += xrdp_orders_get_unicode_bytes(notify_state->infotip.text);
        /* Timeout (4 bytes) */
        /* InfoFlags (4 bytes) */
        order_size += 8;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_STATE)
    {
        /* State (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_ICON)
    {
        /* Icon (variable) */
        use_cmap = 0;

        if ((notify_state->icon_info.bpp == 1) || (notify_state->icon_info.bpp == 2) ||
                (notify_state->icon_info.bpp == 4))
        {
            use_cmap = 1;
        }

        order_size += 12 + notify_state->icon_info.mask_bytes +
                      notify_state->icon_info.data_bytes;

        if (use_cmap)
        {
            order_size += notify_state->icon_info.cmap_bytes + 2;
        }
    }

    if (field_present_flags & WINDOW_ORDER_CACHED_ICON)
    {
        /* CachedIcon (3 bytes) */
        order_size += 3;
    }

    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    out_uint32_le(self->out_s, field_present_flags);
    /* windowId (4 bytes) */
    out_uint32_le(self->out_s, window_id);
    /* notifyIconId (4 bytes) */
    out_uint32_le(self->out_s, notify_id);

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_VERSION)
    {
        /* Version (4 bytes) */
        out_uint32_le(self->out_s, notify_state->version);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_TIP)
    {
        /* ToolTip (variable) UNICODE_STRING */
        xrdp_orders_send_as_unicode(self->out_s, notify_state->tool_tip);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_INFO_TIP)
    {
        /* InfoTip (variable) TS_NOTIFY_ICON_INFOTIP */
        out_uint32_le(self->out_s, notify_state->infotip.timeout);
        out_uint32_le(self->out_s, notify_state->infotip.flags);
        xrdp_orders_send_as_unicode(self->out_s, notify_state->infotip.text);
        xrdp_orders_send_as_unicode(self->out_s, notify_state->infotip.title);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_NOTIFY_STATE)
    {
        /* State (4 bytes) */
        out_uint32_le(self->out_s, notify_state->state);
    }

    if (field_present_flags & WINDOW_ORDER_ICON)
    {
        /* Icon (variable) */
        xrdp_orders_send_ts_icon(self->out_s, notify_state->icon_cache_entry,
                                 notify_state->icon_cache_id,
                                 &notify_state->icon_info);
    }

    if (field_present_flags & WINDOW_ORDER_CACHED_ICON)
    {
        /* CacheEntry (2 bytes) */
        out_uint16_le(self->out_s, notify_state->icon_cache_entry);
        /* CacheId (1 byte) */
        out_uint8(self->out_s, notify_state->icon_cache_id);
    }

    return 0;
}

/*****************************************************************************/
/* RAIL */
/* returns error */
/* used for both Non-Monitored Desktop and Actively Monitored Desktop */
int
xrdp_orders_send_monitored_desktop(struct xrdp_orders *self,
                                   struct rail_monitored_desktop_order *mdo,
                                   int flags)
{
    int order_size;
    int order_flags;
    int field_present_flags;
    int index;

    order_size = 7;
    field_present_flags = flags | WINDOW_ORDER_TYPE_DESKTOP;

    if (field_present_flags & WINDOW_ORDER_FIELD_DESKTOP_ACTIVE_WND)
    {
        /* ActiveWindowId (4 bytes) */
        order_size += 4;
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_DESKTOP_ZORDER)
    {
        /* NumWindowIds (1 byte) */
        order_size += 1;
        /* WindowIds (variable) */
        order_size += mdo->num_window_ids *  4;
    }

    if (xrdp_orders_check(self, order_size) != 0)
    {
        return 1;
    }
    self->order_count++;
    order_flags = TS_SECONDARY;
    order_flags |= 0xb << 2; /* type TS_ALTSEC_WINDOW */
    out_uint8(self->out_s, order_flags);
    /* orderSize (2 bytes) */
    out_uint16_le(self->out_s, order_size);
    /* FieldsPresentFlags (4 bytes) */
    out_uint32_le(self->out_s, field_present_flags);

    if (field_present_flags & WINDOW_ORDER_FIELD_DESKTOP_ACTIVE_WND)
    {
        /* ActiveWindowId (4 bytes) */
        out_uint32_le(self->out_s, mdo->active_window_id);
    }

    if (field_present_flags & WINDOW_ORDER_FIELD_DESKTOP_ZORDER)
    {
        /* NumWindowIds (1 byte) */
        out_uint8(self->out_s, mdo->num_window_ids);

        /* WindowIds (variable) */
        for (index = 0; index < mdo->num_window_ids; index++)
        {
            out_uint32_le(self->out_s, mdo->window_ids[index]);
        }
    }

    return 0;
}
