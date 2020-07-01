#include <stdlib.h>
#include <stdio.h>
#include "vpi_user.h"

PLI_INT32 showvalue_calltf()
{
    // We already know from compiletf that we have 1 arg and it's of the right type.
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle net_handle = vpi_scan(arg_iterator);
    vpi_free_object(arg_iterator);

    // read the value
    s_vpi_value value;
    value.format = vpiBinStrVal;
    vpi_get_value(net_handle, &value);
    vpi_printf("%s: %s\n", vpi_get_str(vpiFullName, net_handle), value.value.str);

    return 0;
}

/**
 * Given a vpiHandle to a vpiSysTfCall (acquired by calling vpi_handle(vpiSysTfCall, NULL)), this
 * function will return an integer telling how many arguments have been passed into the present
 * SysTfCall.
 */
int count_vpi_args(vpiHandle systf_handle)
{
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    if (arg_iterator == NULL) {
        return 0;
    } else {
        int argcount = 0;
        while (vpi_scan(arg_iterator) != NULL) {
            argcount++;
        }
        return argcount;
    }
}

/**
 * compiletf functions make sure that valid parameters are passed in.
 */
PLI_INT32 showvalue_compiletf()
{
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("$showvalue: ERROR: NULL systf handle\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    int numargs = count_vpi_args(systf_handle);
    if (numargs != 1) {
        vpi_printf("$showvalue: ERROR: 1 arg required; %i args found.\n", numargs);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle arg = vpi_scan(arg_iterator);
    PLI_INT32 arg_type = vpi_get(vpiType, arg);
    if ((arg_type != vpiNet) && (arg_type != vpiReg)) {
        vpi_printf("$showvalue: ERROR: arg must be a net or a reg\n");
        vpi_free_object(arg_iterator);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    vpi_free_object(arg_iterator);

    return 0;
}

void showvalue_register()
{
    s_vpi_systf_data tfd;

    tfd.type = vpiSysTask;
    tfd.sysfunctype = 0;
    tfd.tfname = "$showvalue";
    tfd.calltf = showvalue_calltf;
    tfd.compiletf = showvalue_compiletf;
    tfd.sizetf = NULL;
    tfd.user_data = NULL;

    vpi_register_systf(&tfd);
}

void (*vlog_startup_routines[])() =
{
    showvalue_register,
    0
};
