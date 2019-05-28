
/*
 * Copyright (C) 2019-2019 ZengJinji
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_int_t   begin;
    ngx_int_t   end;
} ngx_http_between_time_t;


typedef struct {
    ngx_uint_t      percent;
    ngx_array_t    *times;
} ngx_http_between_loc_conf_t;


static ngx_int_t ngx_http_variable_is_between_time(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_variable_is_between_percent(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_http_between_add_variables(ngx_conf_t *cf);
static void *ngx_http_between_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_between_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);

static char *ngx_http_between_time(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static ngx_command_t  ngx_http_between_commands[] = {

    { ngx_string("between_time"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE123,
      ngx_http_between_time,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("between_percent"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE123,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_between_loc_conf_t, percent),
      NULL },

    ngx_null_command
};


static ngx_http_module_t  ngx_http_between_module_ctx = {
    ngx_http_between_add_variables,           /* preconfiguration */
    NULL,                                     /* postconfiguration */
                                                 
    NULL,                                     /* create main configuration */
    NULL,                                     /* init main configuration */
                                              
    NULL,                                     /* create server configuration */
    NULL,                                     /* merge server configuration */

    ngx_http_between_create_loc_conf,         /* create location configuration */
    ngx_http_between_merge_loc_conf           /* merge location configuration */
};


ngx_module_t  ngx_http_between_module = {
    NGX_MODULE_V1,
    &ngx_http_between_module_ctx,    /* module context */
    ngx_http_between_commands,       /* module directives */
    NGX_HTTP_MODULE,                 /* module type */
    NULL,                            /* init master */
    NULL,                            /* init module */
    NULL,                            /* init process */
    NULL,                            /* init thread */
    NULL,                            /* exit thread */
    NULL,                            /* exit process */
    NULL,                            /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_variable_is_between_time(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_int_t                         hh, mm, now;
    ngx_uint_t                        i;
    ngx_http_between_time_t          *between;
    ngx_http_between_loc_conf_t      *blcf;

    v->len = 1;
    v->valid = 1;
    v->not_found = 0;
    v->no_cacheable = 0;

    blcf = ngx_http_get_module_loc_conf(r, ngx_http_between_module);

    if (blcf->times) {

        v->data = (u_char *) "0";

        hh =  ngx_atoi(ngx_cached_http_log_time.data + 12, 2);
        mm =  ngx_atoi(ngx_cached_http_log_time.data + 15, 2);
        now = hh * 100 + mm;

        between = blcf->times->elts;

        for (i = 0; i < blcf->times->nelts; i++) {
            if (between[i].begin <= now && now <= between[i].end) {
                v->data = (u_char *) "1";
                break;
            }
        }

    } else {
        v->data = (u_char *) "1";
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_variable_is_between_percent(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    ngx_uint_t                        x;
    ngx_http_between_loc_conf_t      *blcf;

    v->len = 1;
    v->valid = 1;
    v->not_found = 0;
    v->no_cacheable = 0;

    blcf = ngx_http_get_module_loc_conf(r, ngx_http_between_module);
    if (blcf->percent == 100) {
        v->data = (u_char *) "1";

    } else {
        x = ngx_random() % 100 + 1;

        if (x <= blcf->percent) {
            v->data = (u_char *) "1";

        } else {
            v->data = (u_char *) "0";
        }
    }

    return NGX_OK;
}


static ngx_http_variable_t  ngx_http_between_vars[] = {

    { ngx_string("is_between_time"), NULL, ngx_http_variable_is_between_time, 0,
      NGX_HTTP_VAR_CHANGEABLE, 0 },

    { ngx_string("is_between_percent"), NULL, ngx_http_variable_is_between_percent, 0,
      NGX_HTTP_VAR_CHANGEABLE, 0 },

      ngx_http_null_variable
};


static ngx_int_t
ngx_http_between_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_between_vars; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}


static void *
ngx_http_between_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_between_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_between_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->times = NULL;
     */

    conf->percent = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *ngx_http_between_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child)
{
    ngx_http_between_loc_conf_t *prev = parent;
    ngx_http_between_loc_conf_t *conf = child;

    if (conf->times == NULL) {
        conf->times = prev->times;
    }

    ngx_conf_merge_uint_value(conf->percent,
                              prev->percent, 100);

    return NGX_CONF_OK;
}


static char *
ngx_http_between_time(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_between_loc_conf_t *blcf = conf;

    ngx_int_t                    begin_hh, begin_mm, end_hh, end_mm;
    ngx_str_t                   *value;
    ngx_uint_t                   i;
    ngx_http_between_time_t     *bt;

    if (blcf->times) {
        return "is duplicate";
    }

    blcf->times = ngx_array_create(cf->pool, 3, sizeof(ngx_http_between_time_t));
    if (blcf->times == NULL) {
        return NGX_CONF_ERROR;
    }

    value = cf->args->elts;

    for (i = 1; i < cf->args->nelts; i++) {

        if (value[i].len != 11 || value[i].data[2] != ':'
            || value[i].data[5] != '-' || value[i].data[8] != ':') 
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid value \"%V\"", &value[i]);
            return NGX_CONF_ERROR;
        }
        
        begin_hh = ngx_atoi(value[i].data + 0, 2);
        begin_mm = ngx_atoi(value[i].data + 3, 2);
        end_hh = ngx_atoi(value[i].data + 6, 2);
        end_mm = ngx_atoi(value[i].data + 9, 2);

        if (begin_hh < 0 || begin_hh > 23
            || begin_mm < 0 || begin_mm > 59
            || end_hh < 0 || end_hh > 23
            || end_mm < 0 || end_mm > 59)
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid value \"%V\"", &value[i]);
            return NGX_CONF_ERROR;
        }

        bt = ngx_array_push(blcf->times);
        if (bt == NULL) {
            return NGX_CONF_ERROR;
        }

        bt->begin = begin_hh * 100 + begin_mm;
        bt->end = end_hh * 100 + end_mm;
    }

    return NGX_CONF_OK;
}
