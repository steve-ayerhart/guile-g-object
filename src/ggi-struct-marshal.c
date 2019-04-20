/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib.h>

#include "ggi-value.h"
#include "ggi-cache.h"
#include "gtype.h"
#include "gvalue.h"
#include "gobject.h"

static gboolean
_is_union_member (GIInterfaceInfo *interface_info, SCM scm_arg)
{
    gint n;
    gint n_fields;
    GIUnionInfo *union_info;
    GIInfoType info_type;
    gboolean is_member = FALSE;

    info_type = g_base_info_get_type (interface_info);

    if (info_type != GI_INFO_TYPE_UNION)
        return FALSE;

    union_info = (GIUnionInfo *) interface_info;
    n_fields = g_union_info_get_n_fields (union_info);

    for (n = 0; n < n_fields; n++)
        {
            GIFieldInfo *field_info;
            GITypeInfo *field_type_info;

            field_info = g_union_info_get_field (union_info, n);
            field_type_info = g_field_info_get_type (field_info);

            if (g_type_info_get_tag (field_type_info) == GI_TYPE_TAG_INTERFACE)
                {
                    GIInterfaceInfo *field_iface_info;
                    SCM scm_type;

                    field_iface_info = g_type_info_get_interface (field_type_info);
                }
        }
}


void
ggi_arg_gvalue_from_scm_cleanup (GGIInvokeState *state,
                                 GGIArgCache    *arg_cache,
                                 SCM             scm_arg,
                                 gpointer        data,
                                 gboolean        was_processed)
{
    
}

static void
arg_gclosure_from_scm_cleanup (GGIInvokeState *state,
                               GGIArgCache    *arg_cache,
                               SCM             scm_arg,
                               gpointer        cleanup_data,
                               gboolean        was_processed)
{
    
}

gboolean
ggi_arg_struct_from_scm_marshal (SCM          scm_arg,
                                 GIArgument  *arg,
                                 const gchar *arg_name,
                                 GIBaseInfo  *interface_info,
                                 GType        g_type,
                                 SCM          scm_type,
                                 GITransfer   transfer,
                                 gboolean     copy_reference,
                                 gboolean     is_foreign,
                                 gboolean     is_pointer)
{
    g_debug ("ggi_arg_struct_from_scm_marshal");

    gboolean is_union = FALSE;

    if (scm_arg == SCM_UNDEFINED) // unbound?
        {
            arg->v_pointer = NULL;
            return TRUE;
        }

    // FIXME: move this large if statement to cache and set correct marshaller

    if (g_type_is_a (g_type, G_TYPE_CLOSURE))
        {
            // TODO:
            //return ggi_arg_gclosure_from_scm_marshal (scm_arg, arg, transfer);
        }
    else if (g_type_is_a (g_type, G_TYPE_VALUE))
        {
            // TODO:
            /*
              return ggi_arg_gvalue_from_scm_marshal (scm_arg,
              arg,
              transfer,
              copy_reference);
            */
        }
    else if (is_foreign)
        {
            SCM success;
            // TODO
            g_critical ("from_scm_marshal TODO");
        }
    else if (SCM_GOBJECTP (scm_arg))
        {
            // TODO
            g_critical ("from_scm_marshal TODO");
        }

    if (g_type_is_a (g_type, G_TYPE_BOXED))
        {
            g_debug ("BOXXXXXXXED");

            if (is_union ||  scm_c_gvalue_holds (scm_arg, g_type))
                {
                    g_debug ("WE GOT A BOX");
                    arg->v_pointer = scm_c_gvalue_peek_boxed (scm_arg);
                    if (transfer == GI_TRANSFER_EVERYTHING)
                        arg->v_pointer = g_boxed_copy (g_type, arg->v_pointer);
                }
            else
                {
                    g_debug ("NO ERRRORRRRRRRR");
                    goto type_error;
                }
        }
    else if (g_type_is_a (g_type, G_TYPE_POINTER) ||
             g_type_is_a (g_type, G_TYPE_VARIANT) ||
             g_type == G_TYPE_NONE)
        {
            g_warn_if_fail (g_type_is_a (g_type, G_TYPE_VARIANT) || !is_pointer || transfer == GI_TRANSFER_NOTHING);

            // TODO: variants
        }
    else
        {
            scm_misc_error ("not implemented",
                            "structure type '~s' is not supported yet",
                            scm_from_locale_string (g_type_name (g_type)));

            return FALSE;
        }

    return TRUE;

 type_error:
    {
        g_debug ("HOO");
        return FALSE;
    }
}

static gboolean
arg_struct_from_scm_marshal_adapter (GGIInvokeState   *state,
                                     GGICallableCache *callable_cache,
                                     GGIArgCache      *arg_cache,
                                     SCM               scm_arg,
                                     GIArgument        *arg,
                                     gpointer          *cleanup_data)
{
    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;

    gboolean res = ggi_arg_struct_from_scm_marshal (scm_arg,
                                                    arg,
                                                    arg_cache->arg_name,
                                                    iface_cache->interface_info,
                                                    iface_cache->g_type,
                                                    iface_cache->scm_type,
                                                    arg_cache->transfer,
                                                    TRUE,
                                                    iface_cache->is_foreign,
                                                    arg_cache->is_pointer);

    *cleanup_data = arg->v_pointer;
    return res;
}

static void
arg_foreign_from_scm_cleanup (GGIInvokeState *state,
                              GGIArgCache    *arg_cache,
                              SCM             scm_arg,
                              gpointer        data,
                              gboolean        was_processed)
{
    // TODO
}

SCM
ggi_gboxed_new (GType boxed_type, gpointer boxed, gboolean copy_boxed, gboolean own_ref)
{
    g_return_val_if_fail (boxed_type != 0, SCM_UNDEFINED);
    //    g_return_val_if_fail (!copy_boxed || (copy_boxed && own_ref), SCM_UNDEFINED);

    SCM scm_boxed;

    if (copy_boxed)
        scm_boxed = scm_c_gvalue_new_take_boxed (boxed_type, boxed);
    else
        scm_boxed = scm_c_gvalue_new_from_boxed (boxed_type, boxed);

    return scm_boxed;
}
static SCM
ggi_arg_struct_to_scm_marshaller (GIArgument      *arg,
                                  GIInterfaceInfo *interface_info,
                                  GType            g_type,
                                  SCM              scm_type,
                                  GITransfer       transfer,
                                  gboolean         is_allocated,
                                  gboolean         is_foreign)
{
    g_debug ("ggi_arg_struct_to_scm_marshaller");

    SCM scm_obj;

    if (arg->v_pointer == NULL)
        return SCM_UNDEFINED;

    if (g_type_is_a (g_type, G_TYPE_VALUE))
        {
            g_debug (" struct is VALUE");
            scm_obj = scm_c_gvalue_to_scm (arg->v_pointer);
        }
    else if (is_foreign)
        {
            // TODO
            g_critical ("is_foreign: not implemented");
        }
    else if (g_type_is_a (g_type, G_TYPE_BOXED))
        {
            g_debug (" struct is BOXED");

            scm_obj = ggi_gboxed_new (g_type,
                                      arg->v_pointer,
                                      transfer == GI_TRANSFER_EVERYTHING || is_allocated,
                                      is_allocated ? g_struct_info_get_size (interface_info) : 0);
        }
    else if (g_type_is_a (g_type, G_TYPE_POINTER))
        {
            g_critical ("g_type_pointer not implemented");
        }
    else if (g_type_is_a (g_type, G_TYPE_VARIANT))
        {
            g_critical ("gtype_variant");
        }
    else if (g_type == G_TYPE_NONE)
        {
            g_critical ("UNIMPLI");
        }
    else
        {
            g_critical ("UNIMPLI");
        }

    return scm_obj;
}

SCM
ggi_arg_struct_to_scm_marshal (GIArgument      *arg,
                               GIInterfaceInfo *interface_info,
                               GType            g_type,
                               SCM              scm_type,
                               GITransfer       transfer,
                               gboolean         is_allocated,
                               gboolean         is_foreign)
{
    g_debug ("ggi_arg_struct_to_scm_marshal");

    SCM scm_value = ggi_arg_struct_to_scm_marshaller (arg,
                                                      interface_info,
                                                      g_type,
                                                      scm_type,
                                                      transfer,
                                                      is_allocated,
                                                      is_foreign);

    //if (scm_value && SCM_GOBJECT_P (scm_value) && transfer == GI_TRANSFER_NOTHING)

    return scm_value;
}

static SCM
arg_struct_to_scm_marshal_adapter (GGIInvokeState   *state,
                                   GGICallableCache *callable_cache,
                                   GGIArgCache      *arg_cache,
                                   GIArgument       *arg,
                                   gpointer         *cleanup_data)
{
    g_debug ("arg_struct_to_scm_marshal_adapter");

    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;
    SCM scm_value;

    scm_value = ggi_arg_struct_to_scm_marshaller (arg,
                                                  iface_cache->interface_info,
                                                  iface_cache->g_type,
                                                  iface_cache->scm_type,
                                                  arg_cache->transfer,
                                                  arg_cache->is_caller_allocates,
                                                  iface_cache->is_foreign);

    *cleanup_data = scm_value;

    return scm_value;
}

static void
arg_foreign_to_scm_cleanup (GGIInvokeState *state,
                            GGIArgCache    *arg_cache,
                            gpointer        cleanup_data,
                            gpointer        data,
                            gboolean        was_processed)
{
    // TODO
}

static void
arg_boxed_to_scm_cleanup(GGIInvokeState *state,
                         GGIArgCache    *arg_cache,
                         gpointer        cleanup_data,
                         gpointer        data,
                         gboolean        was_processed)
{
    // TODO
}

static gboolean
arg_type_class_from_scm_marshal (GGIInvokeState   *state,
                                 GGICallableCache *callable_cache,
                                 GGIArgCache      *arg_cache,
                                 SCM               scm_arg,
                                 GIArgument       *arg,
                                 gpointer         *cleanup_data)
{
    GType gtype= scm_c_gtype_class_to_gtype (scm_arg);

    if (G_TYPE_IS_CLASSED (gtype))
        {
            arg->v_pointer = g_type_class_ref (gtype);
            *cleanup_data = arg->v_pointer;
            return TRUE;
        }
    else
        {
            // TODO SCM ERROR
            g_critical ("NOT A GTYPE");
            return FALSE;
        }
}

static void
arg_type_class_from_scm_cleanup (GGIInvokeState *state,
                                 GGIArgCache    *arg_cache,
                                 SCM             scm_arg,
                                 gpointer        data,
                                 gboolean        was_processed)
{
    if (was_processed)
        g_type_class_unref (data);
}

static void
arg_struct_from_scm_setup (GGIArgCache     *arg_cache,
                           GIInterfaceInfo *iface_info,
                           GITransfer       transfer)
{
    g_debug ("arg_struct_from_scm_setup");

    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;

    if (g_struct_info_is_gtype_struct ((GIStructInfo *) iface_info))
        {
            arg_cache->from_scm_marshaller = arg_type_class_from_scm_marshal;

            if (transfer == GI_TRANSFER_NOTHING)
                arg_cache->from_scm_cleanup = arg_type_class_from_scm_cleanup;
        }
    else
        {
            arg_cache->from_scm_marshaller = arg_struct_from_scm_marshal_adapter;

            if (g_type_is_a (iface_cache->g_type, G_TYPE_CLOSURE))
                {
                    arg_cache->from_scm_cleanup = arg_gclosure_from_scm_cleanup;
                }
            else if (iface_cache->g_type == G_TYPE_VALUE)
                {
                    arg_cache->from_scm_cleanup = ggi_arg_gvalue_from_scm_cleanup;
                }
            else if (iface_cache->is_foreign)
                {
                    arg_cache->from_scm_cleanup = arg_foreign_from_scm_cleanup;
                }
        }
}

static void
arg_struct_to_scm_setup (GGIArgCache      *arg_cache,
                         GIInterfaceInfo  *iface_info,
                         GITransfer        transfer)
{
    g_debug ("arg_struct_to_scm_setup");

    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;

    if (arg_cache->to_scm_marshaller == NULL)
        {
            arg_cache->to_scm_marshaller = arg_struct_to_scm_marshal_adapter;
        }

    iface_cache->is_foreign = g_struct_info_is_foreign ((GIStructInfo *) iface_info);

    if (iface_cache->is_foreign)
        arg_cache->to_scm_cleanup = arg_foreign_to_scm_cleanup;
    else if (!g_type_is_a (iface_cache->g_type, G_TYPE_VALUE) &&
             g_type_is_a (iface_cache->g_type, G_TYPE_BOXED))
        arg_cache->to_scm_cleanup = arg_boxed_to_scm_cleanup;

}

GGIArgCache *
ggi_arg_struct_new_from_info (GITypeInfo      *type_info,
                              GIArgInfo       *arg_info,
                              GITransfer       transfer,
                              GGIDirection     direction,
                              GIInterfaceInfo *iface_info)
{
    g_debug ("ggi_arg_struct_new_from_info");

    GGIArgCache *cache = NULL;
    GGIInterfaceCache *iface_cache;

    cache = ggi_arg_interface_new_from_info (type_info,
                                             arg_info,
                                             transfer,
                                             direction,
                                             iface_info);

    if (cache == NULL)
        return NULL;

    iface_cache = (GGIInterfaceCache *) cache;
    iface_cache->is_foreign = (g_base_info_get_type ((GIBaseInfo *) iface_info) == GI_INFO_TYPE_STRUCT &&
                               g_struct_info_is_foreign ((GIStructInfo *) iface_info));

    if (direction & GGI_DIRECTION_FROM_SCM)
        arg_struct_from_scm_setup (cache, iface_info, transfer);

    if (direction & GGI_DIRECTION_TO_SCM)
        arg_struct_to_scm_setup (cache, iface_info, transfer);

    return cache;
}
