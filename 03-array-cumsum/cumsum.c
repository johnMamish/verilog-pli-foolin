#include <stdlib.h>
#include <stdio.h>
#include "vpi_user.h"

PLI_INT32 cumsum_calltf()
{
    // We already know from compiletf that we have 2 args and they're both of the right type
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle array_in_handle = vpi_scan(arg_iterator);
    vpiHandle array_out_handle = vpi_scan(arg_iterator);
    vpi_free_object(arg_iterator);

    // check the sizes of the 2 arrays
    PLI_INT32 in_size  = vpi_get(vpiSize, array_in_handle);
    PLI_INT32 out_size = vpi_get(vpiSize, array_out_handle);
    vpi_printf("arg a size is %i, arg b size is %i\n", in_size, out_size);
    if (out_size != in_size) {
        vpi_printf("$cumsum: ERROR: argument arrays must have same size.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    int32_t accum = 0;
    vpiHandle array_in_iterator = vpi_iterate(vpiMemoryWord, array_in_handle);
    vpiHandle array_out_iterator = vpi_iterate(vpiMemoryWord, array_out_handle);
    vpiHandle array_in_element;
    vpiHandle array_out_element;
    while(((array_in_element = vpi_scan(array_in_iterator)) != NULL) &&
          ((array_out_element = vpi_scan(array_out_iterator)) != NULL)) {
        s_vpi_value in_value;
        in_value.format = vpiIntVal;
        vpi_get_value(array_in_element, &in_value);
        accum += in_value.value.integer;
        vpi_printf("$cumsum: TRACE: accumulating a value of %i\n", in_value.value.integer);

        s_vpi_value out_value;
        out_value.format = vpiIntVal;
        out_value.value.integer = accum;
        vpi_put_value(array_out_element, &out_value, NULL, vpiNoDelay);
    }

    return 0;
}

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
PLI_INT32 cumsum_compiletf()
{
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("$cumsum: ERROR: NULL systf handle\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    int numargs = count_vpi_args(systf_handle);
    if (numargs != 2) {
        vpi_printf("$cumsum: ERROR: 2 args required; %i args found.\n", numargs);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle arg;
    while((arg = vpi_scan(arg_iterator)) != NULL) {
        PLI_INT32 arg_type = vpi_get(vpiType, arg);
        vpi_printf("%s\n", vpi_get_str(vpiType, arg));

        if (arg_type != vpiMemory) {
            vpi_printf("$cumsum: ERROR: arg must have type vpiMemory.\n");
            vpi_free_object(arg_iterator);
            vpi_control(vpiFinish, 0);
            return 0;
        }
    }

    return 0;
}

void cumsum_register()
{
    s_vpi_systf_data tfd;

    tfd.type = vpiSysTask;
    tfd.sysfunctype = 0;
    tfd.tfname = "$cumsum";
    tfd.calltf = cumsum_calltf;
    tfd.compiletf = cumsum_compiletf;
    tfd.sizetf = NULL;
    tfd.user_data = NULL;

    vpi_register_systf(&tfd);
}

void (*vlog_startup_routines[])() =
{
    cumsum_register,
    0
};
