/* -*- Mode: C; c-basic-offset: 4 -*-
 *
 */

#include <libguile.h>
#include "ggi-basic-types.h"
#include "ggi-argument.h"
#include "gtype.h"

#if defined(G_OS_WIN32)
#include <float.h>
static boolean
ggi_is_finite (gdouble value)
{
    return _finite(value);
}
#else
#include <math.h>
static gboolean
ggi_is_finite (double value)
{
    return isfinite (value);
}
#endif

static gboolean
marshal_from_scm_void (GGIInvokeState   *state,
                       GGICallableCache *callable_cache,
                       GGIArgCache      *arg_cache,
                       SCM               scm_arg,
                       GIArgument       *arg,
                       gpointer         *cleanup_data)
{
    g_warn_if_fail (arg_cache->transfer == GI_TRANSFER_NOTHING);

    if (ggi_gpointer_from_scm (scm_arg, &(arg->v_pointer)))
        {
            *cleanup_data = arg->v_pointer;
            return TRUE;
        }

    return FALSE;
}

static gpointer
ggi_scm_to_gpointer (SCM scm_arg)
{
    if (scm_is_false (scm_arg))
        {
            return NULL;
        }
    else if (scm_is_exact_integer (scm_arg))
        {
            return (gpointer) scm_to_uintptr_t (scm_arg);
        }
    else if (scm_is_bytevector (scm_arg) && SCM_BYTEVECTOR_LENGTH (scm_arg) >= 8)
        {
            return SCM_BYTEVECTOR_CONTENTS (scm_arg);
        }
    else
        {
            scm_misc_error ("type marshal",
                            "cannot convert '~a' to gpointer",
                            scm_list_1 (scm_arg));
        }
}


static gboolean
ggi_scm_to_gdouble (SCM scm_value, gdouble *gdouble_)
{
    if (scm_is_false (scm_number_p (scm_value)))
        {
            scm_misc_error ("type marshal",
                            "Must be a number, not '~a'",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    *gdouble_ = scm_to_double (scm_value);

    return TRUE;
}

static gboolean
ggi_scm_to_gunichar (SCM scm_value, gunichar *gunichar_)
{
    // TODO: handle bytevector ?
    if (scm_is_true (scm_char_p (scm_value)))
        {
            *gunichar_ = (gunichar) scm_to_uint32 (scm_char_to_integer (scm_value));

            return TRUE;
        }
    else if (scm_is_true (scm_string_p (scm_value)))
        {
            SCM scm_char_list = scm_string_to_list (scm_value);

            if (1 == scm_to_long (scm_length (scm_char_list)))
                {
                    *gunichar_ = (gunichar) scm_to_uint32 (scm_char_to_integer (scm_list_ref (scm_value, 0)));

                    return TRUE;
                }
            else
                {
                    scm_misc_error ("type marshal",
                                    "Must be a 1 character string, not ~d",
                                    scm_list_1 (scm_length (scm_char_list)));

                    return FALSE;
                }
        }
    else if (scm_is_exact_integer (scm_value))
        {
            // TODO: assuming this tests code points. find better way?
            *gunichar_ = (gunichar) scm_to_uint32 (scm_char_to_integer (scm_integer_to_char (scm_value)));

            return TRUE;
        }
    else
        {
            scm_misc_error ("type marshal",
                            "cannot convert  '~a' to guinchar",
                            scm_list_1 (scm_value));

            return FALSE;
        }
}

static gboolean
ggi_scm_to_gfloat (SCM scm_value, gfloat *gfloat_)
{
    gdouble double_;

    if (scm_is_false (scm_number_p (scm_value)))
        {
            scm_misc_error ("type marshal",
                            "Must be a number, not '~a'",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    double_ = scm_to_double (scm_value);

    if (ggi_is_finite (double_) && (double_ < -G_MAXFLOAT || double_ > G_MAXFLOAT))
        {
            scm_misc_error ("type marshal",
                            "Cannot convert '~a' to gfloat, out of range",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    *gfloat_ = (gfloat) double_;

    return TRUE;
}

static gboolean
ggi_scm_to_gtype (SCM scm_value, GType *gtype_)
{
    if (!SCM_GTYPE_CLASSP (scm_value))
        {
            scm_misc_error ("type marshal",
                            "Must be a guile Gtype, not '~a'",
                            scm_list_1 (scm_value));
            return FALSE;
        }

    *gtype_ = scm_c_gtype_class_to_gtype (scm_value);

    return TRUE;
}


// marshalling to scheme is relatively straight forward.
SCM
ggi_marshal_to_scm_basic_type (GIArgument *arg,
                               GITypeTag   type_tag)
{
    switch (type_tag)
        {
        case GI_TYPE_TAG_BOOLEAN:
            return scm_from_bool (arg->v_boolean);

        case GI_TYPE_TAG_INT8:
            return scm_from_int8 (arg->v_int8);

        case GI_TYPE_TAG_UINT8:
            return scm_from_uint8 (arg->v_uint8);

        case GI_TYPE_TAG_INT16:
            return scm_from_int16 (arg->v_int16);

        case GI_TYPE_TAG_UINT16:
            return scm_from_uint16 (arg->v_uint16);

        case GI_TYPE_TAG_INT32:
            return scm_from_int32 (arg->v_int32);

        case GI_TYPE_TAG_UINT32:
            return scm_from_uint32 (arg->v_uint32);

        case GI_TYPE_TAG_INT64:
            return scm_from_int64 (arg->v_int64);

        case GI_TYPE_TAG_UINT64:
            return scm_from_uint64 (arg->v_uint64);

        case GI_TYPE_TAG_FLOAT:
            return scm_from_double ((double) arg->v_float);

        case GI_TYPE_TAG_DOUBLE:
            return scm_from_double (arg->v_double);

        case GI_TYPE_TAG_GTYPE:
            return scm_c_gtype_lookup_class (arg->v_size);

        case GI_TYPE_TAG_UNICHAR:
            return scm_integer_to_char (scm_from_uint32 (arg->v_uint32));

        case GI_TYPE_TAG_UTF8:
            if (!arg->v_string)
                return scm_c_make_string(0, SCM_MAKE_CHAR(0));
            else
                return scm_from_utf8_string (arg->v_string);

        case GI_TYPE_TAG_FILENAME:
            if (!arg->v_string)
                return scm_c_make_string(0, SCM_MAKE_CHAR(0));
            else
                return scm_from_locale_string (arg->v_string);

        default:
            scm_misc_error("argument marshal",
                           "Type tag %d not supported",
                           scm_list_1 (scm_from_size_t (type_tag)));
            return SCM_UNSPECIFIED;
        }
}
