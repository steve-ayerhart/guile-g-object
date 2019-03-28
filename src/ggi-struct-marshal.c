/* -*- Mode: C; c-basic-offset: 4 -*-
 * vim: tabstop=4 shiftwidth=4 expandtab
 */

#include <libguile.h>
#include <glib.h>

#include "ggi-cache.h"
#include "gtype.h"
#include "gvalue.h"

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
    gboolean is_union = FALSE;

    if (scm_arg == SCM_UNSPECIFIED) // unbound?
        {
            arg->v_pointer = NULL;
            return TRUE;
        }

    return TRUE;
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

static SCM
ggi_arg_struct_to_scm_marshaller (GIArgument      *arg,
                                  GIInterfaceInfo *interface_info,
                                  GType            g_type,
                                  SCM              scm_type,
                                  GITransfer       transfer,
                                  gboolean         is_allocated,
                                  gboolean         is_foreign)
{
    SCM scm_obj;

    if (arg->v_pointer == NULL)
        return SCM_UNDEFINED;

    if (g_type_is_a (g_type, G_TYPE_VALUE))
        {
            scm_obj = scm_c_gvalue_to_scm (arg->v_pointer);
        }
    else if (is_foreign)
        {
            // TODO
            g_critical ("is_foreign: not implemented");
        }
    else if (g_type_is_a (g_type, G_TYPE_BOXED))
        {
            // TODO is this right? (probably not)
            scm_obj = scm_c_gtype_to_class (g_type);
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
    // TODO
}

static void
arg_struct_to_scm_setup (GGIArgCache      *arg_cache,
                         GIInterfaceInfo  *iface_info,
                         GITransfer        transfer)
{
    GGIInterfaceCache *iface_cache = (GGIInterfaceCache *) arg_cache;

    if (arg_cache->to_scm_marshaller == NULL)
        arg_cache->to_scm_marshaller = arg_struct_to_scm_marshal_adapter;

    iface_cache->is_foreign = g_struct_info_is_foreign ((GIStructInfo *) iface_info);

    /*
      TODO
      if (iface_cache->is_foreign)
      arg_cache->to_scm_cleanup = arg_foreign_to_scm_cleanup;
      else if (!g_type_is_a (iface_cache->g_type, G_TYPE_VALUE) &&
      iface_cache->scm_type &&
      g_type_is_a (iface_cache->g_type, G_TYPE_BOXED))
      arg_cache->to_scm_cleanup = arg_boxed_to_scm_cleanup;
    */
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
