/*
*
* Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: ISC
*
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>  /* atoi() */
#include <libubus.h> /* ubus_connect() */
#include <unistd.h>  /* access() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "dbg_if.h"
#include "data_model.h"
#include "notif.h"
#include "onu_hal_macros.h"
#include "onu_hal_trace.h"

static const char* s_ubus_socket_path = "/var/run/ubus/ubus.sock";

static struct ubus_context* s_ctx = NULL;

static void usage(const char* prog) {
    printf("%s -h         : show this help and exit\n", prog);
    printf("%s -d         : start as daemon\n", prog);
    printf("%s            : start in foreground\n", prog);
    printf("\n");
    printf("Examples:\n");
    printf("%s 2       : advertise xpon_onu.2 on ubus\n", prog);
    printf("%s            : run in foreground mode\n", prog);
    printf("%s -d         : run as daemon in background\n", prog);
}

static int daemonize(void) {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        SAH_TRACE_ERROR("Failed to fork daemon process");
        return -1;
    }
    
    /* If we got a good PID, then we can exit the parent process */
    if (pid > 0) {
        printf("onu_hal_agent daemon started with PID: %d\n", pid);
        exit(0);
    }

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        SAH_TRACE_ERROR("Failed to create new session");
        return -1;
    }

    if ((chdir("/")) < 0) {
        SAH_TRACE_ERROR("Failed to change working directory to /");
        return -1;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("/dev/null", O_RDONLY); 
    open("/dev/null", O_WRONLY); 
    open("/dev/null", O_WRONLY);

    return 0;
}

static struct ubus_context* connect_to_ubus(void) {
    struct ubus_context* ctx = NULL;
    if(access(s_ubus_socket_path, F_OK) != 0) {
        s_ubus_socket_path = "/var/run/ubus.sock";
        if(access(s_ubus_socket_path, F_OK) != 0) {
            SAH_TRACE_ERROR("No ubus socket path found");
            return NULL;
        }
    }

    ctx = ubus_connect(s_ubus_socket_path);
    if(!ctx) {
        SAH_TRACE_ERROR("Failed to connect to %s", s_ubus_socket_path);
        return NULL;
    }
    return ctx;
}

int main(int argc, char* argv[]) {

    int onu_nr = 1;  /* Fixed to ONU instance 1 */
    int daemon_mode = 0;
    
    if(argc == 2) {
        const char* option = argv[1];
        if(strncmp(option, "-h", 2) == 0) {
            usage(argv[0]);
            return 0;
        } else if(strncmp(option, "-d", 2) == 0) {
            daemon_mode = 1;
        } else {
	    onu_nr = atoi(option);
            if((onu_nr < 1) || (onu_nr > 2)) {
                printf("Error: invalid ONU nr: %d is not in [1, 2]\n", onu_nr);
                usage(argv[0]);
		return 1; }
        }
    } else if(argc > 2) {
        printf("Error: too many arguments\n");
        usage(argv[0]);
        return 1;
    }

    /* Daemon mode if requested */
    if(daemon_mode) {
        if(daemonize() < 0) {
            return 1;
        }
    }

    SAH_TRACE_INFO("Starting %s with onu_nr=%d %s", argv[0], onu_nr, 
                   daemon_mode ? "(daemon mode)" : "(foreground mode)");

    s_ctx = connect_to_ubus();
    if(!s_ctx) {
        return 1;
    }
    SAH_TRACE_INFO("Connected to %s", s_ubus_socket_path);

    dbg_if_init();

    if(dm_init(s_ctx, onu_nr)) {
        return 1;
    }

    notif_init(s_ctx);

    uloop_init();
    signal(SIGPIPE, SIG_IGN);

    ubus_add_uloop(s_ctx);

    uloop_run();

    SAH_TRACE_INFO("Stopping");
    dbg_if_cleanup();
    dm_cleanup();

    ubus_free(s_ctx);
    uloop_done();

    return 0;
}
