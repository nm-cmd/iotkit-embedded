/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "iotx_utils.h"
#include "iotx_system.h"
#include "sdk-impl_internal.h"

static sdk_impl_ctx_t g_sdk_impl_ctx = {0};

static sdk_impl_ctx_t* _iotx_sdk_impl_get_ctx(void)
{
    return &g_sdk_impl_ctx;
}

void IOT_OpenLog(const char *ident)
{
    const char         *mod = ident;

    if (NULL == mod) {
        mod = "---";
    }

    LITE_openlog(mod);
}

void IOT_CloseLog(void)
{
    LITE_closelog();
}

void IOT_SetLogLevel(IOT_LogLevel level)
{
    LOGLEVEL            lvl = (LOGLEVEL)level;

    if (lvl > LOG_DEBUG_LEVEL) {
        sdk_err("Invalid input level: %d out of [%d, %d]", level,
                LOG_EMERG_LEVEL,
                LOG_DEBUG_LEVEL);
        return;
    }

    LITE_set_loglevel(lvl);
}

void IOT_DumpMemoryStats(IOT_LogLevel level)
{
    LOGLEVEL            lvl = (LOGLEVEL)level;

    if (lvl > LOG_DEBUG_LEVEL) {
        lvl = LOG_DEBUG_LEVEL;
        sdk_warning("Invalid input level, using default: %d => %d", level, lvl);
    }

    LITE_dump_malloc_free_stats(lvl);
}

#if defined(MQTT_COMM_ENABLED)
int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr)
{
    int                 rc = 0;

    if (!info_ptr) {
        sdk_err("Invalid argument, info_ptr = %p", info_ptr);
        return -1;
    }

    STRING_PTR_SANITY_CHECK(product_key, -1);
    STRING_PTR_SANITY_CHECK(device_name, -1);
    STRING_PTR_SANITY_CHECK(device_secret, -1);

    iotx_device_info_init();
    iotx_device_info_set(product_key, device_name, device_secret);

    if (0 == iotx_guider_auth_get())
        rc = iotx_guider_authenticate();
    if (rc == 0) {
        iotx_guider_auth_set(1);
        *info_ptr = (void *)iotx_conn_info_get();
    } else {
        iotx_guider_auth_set(0);
        *info_ptr = NULL;
    }

    return rc;
}

#endif  /* #if defined(MQTT_COMM_ENABLED)   */

int IOT_Ioctl(int option, void *data)
{
    int res = SUCCESS_RETURN;
    sdk_impl_ctx_t *ctx = _iotx_sdk_impl_get_ctx();

    if (option < 0 || data == NULL) {
        sdk_err("Invalid Parameter");
        return FAIL_RETURN;
    }

    switch (option)
    {
        case IOTX_IOCTL_OPT_SET_DOMAIN_TYPE:
        {
            int domain_type = *(int *)data;
            iotx_guider_set_domain_type(domain_type);

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_OPT_SET_DYNAMIC_REGISTER:
        {
            ctx->dynamic_register = *(int *)data;

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_OPT_GET_DYNAMIC_REGISTER:
        {
            *(int *)data = ctx->dynamic_register;

            res = SUCCESS_RETURN;
        }
        break;
        default:
        {
            sdk_err("Unknown Ioctl Option");
            res = FAIL_RETURN;
        }
        break;
    }

    return res;
}