/* -*- Mode: C; c-basic-offset: 4 -*-
 */
#ifndef __GGI_BASIC_TYPES_H__
#define __GGI_BASIC_TypEs_H__

#include <girepository.h>
#include "ggi-cache.h"

G_BEGIN_DECLS

gboolean
ggi_marshal_from_scm_basic_type (SCM         object,         /* in  */
                                 GIArgument *arg,            /* out */
                                 GITypeTag   type_tag,
                                 GITransfer  transfer,
                                 gpointer   *cleanup_data);

SCM
ggi_marshal_to_scm_basic_type (GIArgument *arg,
                               GITypeTag   type_tag,
                               GITransfer  transfer);

GGIArgCache *
ggi_arg_basic_type_new_from_info (GITypeInfo  *type_info,
                                  GIArgInfo   *arg_info,   /* may be null */
                                  GITransfer   transfer,
                                  GGIDirection direction);

SCM ggi_gint64_to_scm (gint64 value);
SCM ggi_guint64_to_scm (guint64 value);
SCM ggi_gfloat_to_scm (gfloat value);
SCM ggi_gdouble_to_scm (gdouble value);
SCM ggi_gboolean_to_scm (gboolean value);
SCM ggi_gint8_to_scm (gint8 value);
SCM ggi_guint8_to_scm (guint8 value);
SCM ggi_gutf8_to_scm (gutf8 value);
SCM ggi_gint_to_scm (gint value);
SCM ggi_glong_to_scm (glong value);
SCM ggi_guint_to_scm (guint value);
SCM ggi_gulong_to_scm (gulong value);
SCM ggi_gfilename_to_scm (gfilename value);
SCM ggi_gsize_to_scm (gsize value);
SCM ggi_gssize_to_scm (g value);
SCM ggi_guint32_to_scm (guint32 value);

gboolean ggi_gint64_from_scm (SCM scm_value, gint64 *result);
gboolean ggi_guint64_from_scm (SCM scm_value, guint64 *result);
gboolean ggi_gfloat_from_scm (SCM scm_value, gfloat *result);
gboolean ggi_gdouble_from_scm (SCM scm_value, gdouble *result);
gboolean ggi_gboolean_from_scm (SCM scm_value, gboolean *result);
gboolean ggi_gint8_from_scm (SCM scm_value, gint8 *result);
gboolean ggi_guint8_from_scm (SCM scm_value, guint8 *result);
gboolean ggi_gutf8_from_scm (SCM scm_value, gutf8 *result);
gboolean ggi_gint_from_scm (SCM scm_value, gint *result);
gboolean ggi_glong_from_scm (SCM scm_value, glong *result);
gboolean ggi_guint_from_scm (SCM scm_value, guint *result);
gboolean ggi_gulong_from_scm (SCM scm_value, gulong *result);
gboolean ggi_gfilename_from_scm (SCM scm_value, gfilename *result);
gboolean ggi_gsize_from_scm (SCM scm_value, gsize *result);
gboolean ggi_gssize_from_scm (SCM scm_value, gssize *result);
gboolean ggi_guint32_from_scm (SCM scm_value, guint32 *result);

G_END_DECLS

#endif