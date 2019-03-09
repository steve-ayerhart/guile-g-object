/* -*- Mode: C; c-basic-offset: 4 -*-
 */

#include <libguile.h>

#include "ggi-invoke.h"
#include "ggi-infos.h"
#include "ggi-cache.h"

extern SCM _GGIDefaultArgPlaceholder;

/* To reduce calls to g_slice_*() we (1) allocate all the memory depended on
 * the argument count in one go and (2) keep one version per argument count
 * around for faster reuse.
 */

#define GGI_INVOKE_ARG_STATE_SIZE(n)   (n * (sizeof (GGIInvokeArgState) + sizeof (GIArgument *)))
#define GGI_INVOKE_ARG_STATE_N_MAX     10
gpointer free_arg_state[GGI_INVOKE_ARG_STATE_N_MAX];

gboolean
_ggi_invoke_arg_state_init (GGIInvokeState *state)
{
    gpointer mem;
}

static SCM
_scm_args_combine_and_check_length (GGICallableCache *cache,
                                    SCM scm_args,
                                    SCM scm_kwargs)
{
    return scm_args;
}

static gboolean
_invoke_state_init_from_cache (GGIInvokeState *state,
                               GGIFunctionCache *function_cache,
                               SCM scm_args,
                               SCM scm_kwargs)
{
    GGICallableCache *cache = (GGICallableCache *) function_cache;

    state->n_args =_ggi_callable_cache_args_len (cache);

    if (cache->throws)
        state->n_args++;

    if (state->function_ptr == NULL)
        state->function_ptr = function_cache->invoker.native_address;

    state->scm_in_args = _scm_args_combine_and_check_length (cache,
                                                             scm_args,
                                                             scm_kwargs);

    if (state->scm_in_args == NULL)
        return FALSE;

    state->n_scm_in_args = scm_to_ssize_t (scm_length (state->scm_in_args));

    if (!_ggi_invoke_arg_state_init (state))
        return FALSE;

    state->error = NULL;

    if (cache->throws)
        {
            gssize error_index = state->n_args - 1;
            // Gerror needs to be a triple pointer
            state->args[error_index].arg_pointer.v_pointer = &state->error;
            state->ffi_args[error_index] = &(state->args[error_index].arg_pointer);
        }

    return TRUE;
}

static gboolean
_invoke_marshal_in_args (GGIInvokeState *state, GGIFunctionCache *function_cache)
{
    GGICallableCache *cache = (GGICallableCache *) function_cache;
    gssize i;

    if (state->n_scm_in_args > cache->n_scm_args)
        return FALSE;
}

static SCM
_invoke_marshal_out_args (GGIInvokeState *state, GGIFunctionCache *function_cache)
{
    GGICallableCache *cache = (GGICallableCache *) function_cache;
    SCM scm_out = SCM_EOL;
    SCM scm_return = SCM_UNSPECIFIED;
    gssize n_out_args = cache->n_to_scm_args - cache->n_to_scm_child_args;

    if (cache->return_cache)
        {
            if (!cache->return_cache->is_skipped)
                {
                    gpointer cleanup_data= NULL;
                    scm_return = cache->return_cache->to_scm_marshaller ( state,
                                                                          cache,
                                                                          cache->return_cache,
                                                                         &state->return_arg,
                                                                         &cleanup_data);
                    state->to_scm_return_arg_cleanup_data = cleanup_data;
                    if (scm_return == SCM_UNSPECIFIED)
                        {
                            // cleanup
                            return SCM_UNSPECIFIED;
                        }
                }
            else
                {
                    if (cache->return_cache->transfer == GI_TRANSFER_EVERYTHING)
                        {
                            // really cleanup
                        }
                }
        }

    if (n_out_args == 0)
        {
            if (cache->return_cache->is_skipped &&  state->error == NULL)
                {
                    g_assert ("no return and no out args?");
                    scm_return = SCM_UNDEFINED;
                }

            scm_out = scm_return;
        }
    else if (!cache->has_return && n_out_args == 1) // 1 out arg and no return
        {
            GGIArgCache *arg_cache = (GGIArgCache *) cache->to_scm_args->data;
            gpointer cleanup_data = NULL;
            scm_out = arg_cache->to_scm_marshaller (state,
                                                    cache,
                                                    arg_cache,
                                                    state->args[arg_cache->c_arg_index].arg_pointer.v_pointer,
                                                    &cleanup_data);
            state->args[arg_cache->c_arg_index].to_scm_arg_cleanup_data = cleanup_data;

            if (scm_out == SCM_UNSPECIFIED)
                {
                    // dunno
                    return SCM_UNDEFINED;
                }
        }
    else // return values
        {
            gssize scm_arg_index = 0;
            GSList *cache_item = cache->to_scm_args;
            gssize num_values = cache->has_return + n_out_args;


            // make list and return values
            // TODO: perhaps use scm_c_values?

            scm_out = SCM_EOL;

            if (cache->has_return)
                {
                    scm_out = scm_append (scm_list_2 (scm_out, scm_return));
                    scm_arg_index++;
                }

            for (; scm_arg_index < num_values; scm_arg_index++)
                {
                    GGIArgCache *arg_cache = (GGIArgCache *) cache_item->data;
                    gpointer cleanup_data = NULL;

                    SCM scm_value = arg_cache->to_scm_marshaller (state,
                                                                  cache,
                                                                  arg_cache,
                                                                  state->args[arg_cache->c_arg_index].arg_pointer.v_pointer,
                                                                  &cleanup_data);
                    state->args[arg_cache->c_arg_index].to_scm_arg_cleanup_data = cleanup_data;

                    if (scm_value == SCM_UNSPECIFIED)
                        {
                            if (cache->has_return)
                                scm_arg_index--;

                            // cleanup

                            // dunno
                            return SCM_UNDEFINED;
                        }

                    scm_out = scm_append (scm_list_2 (scm_out, scm_value));
                    cache_item = cache_item->next;
                }
            return scm_out;
        }
}

SCM
ggi_invoke_c_callable (GGIFunctionCache *function_cache,
                       GGIInvokeState *state,
                       SCM scm_args,
                       SCM scm_kwargs)
{
    GGICallableCache *cache = (GGICallableCache *) function_cache;
    GIFFIReturnValue ffi_return_value= {0};
    SCM scm_return_value = SCM_UNSPECIFIED;

    if (!_invoke_state_init_from_cache (state, function_cache, scm_args, scm_kwargs))
        goto err;

    if (!_invoke_marshal_in_args (state, function_cache))
        goto err;

    ffi_call (&function_cache->invoker.cif,
              state->function_ptr,
              (void *) &ffi_return_value,
              (void **) state->ffi_args);

    if (state->error != NULL)
        {
            // TODO: really handle
            goto err;
        }

    if (cache->return_cache)
        gi_type_info_extract_ffi_return_value (cache->return_cache->type_info,
                                               &ffi_return_value,
                                               &state->return_arg);

    scm_return_value = _invoke_marshal_out_args (state, function_cache);
    // cleanup


 err:

    return scm_return_value;
}

SCM
ggi_callable_info_invoke (GIBaseInfo *info,
                          SCM scm_args,
                          SCM scm_kwargs,
                          GGICallableCache *cache,
                          gpointer user_data)
{
    return ggi_function_cache_invoke ((GGIFunctionCache *) cache,
                               scm_args,
                               scm_kwargs);
}

SCM
_wrap_g_callable_info_invoke (SCM callable_info,
                              SCM scm_args,
                              SCM scm_kwargs)
{
    GIBaseInfo *base_info = ggi_object_get_gi_info (callable_info);

    if (SCM_UNBNDP (scm_foreign_object_ref (callable_info, 1)))
        {
            GGIFunctionCache *function_cache;
            GIInfoType type = g_base_info_get_type (base_info);

            if (type == GI_INFO_TYPE_FUNCTION)
                {
                    GIFunctionInfoFlags flags;

                    flags = g_function_info_get_flags ((GIFunctionInfo *) base_info);

                    if (flags & GI_FUNCTION_IS_CONSTRUCTOR)
                        {
                            //function_cache = ggi_constructor_cache_new (base_info);
                        }
                    else if (flags & GI_FUNCTION_IS_METHOD)
                        {
                            //function_cache = ggi_method_cache_new (base_info);
                        }
                    else
                        {
                            function_cache = ggi_function_cache_new (base_info);
                        }
                }
            else if (type == GI_INFO_TYPE_VFUNC)
                {
                    //function_cache = gg_vfunc_cache_new (base_info);
                }
            else if (type == GI_INFO_TYPE_CALLBACK)
                {
                    g_error ("Cannot invoke callback types");
                }
            else
                {
                    //function_cache = ggi_method_cache_new (base_info);
                }



            scm_foreign_object_set_x (callable_info, 1,
                                      scm_from_pointer ((GGICallableCache *) function_cache, NULL));

            if (function_cache == NULL)
                {
                    return SCM_UNSPECIFIED;
                }
        }

    return ggi_callable_info_invoke (base_info,
                                     scm_args, scm_kwargs,
                                     (GGICallableCache *) scm_to_pointer (scm_foreign_object_ref (callable_info, 1)),
                                     NULL);
}
