#include <stdlib.h>
#include <stdio.h>
#include "vpi_user.h"

/**
 * This function takes a variable number of arguments and prints the variable name of each argument
 * as well as which vpiType it is. For instance, the following verilog:
 *
 *     reg [7:0] myreg;
 *     integer   i;
 *     $print_vpitype(myreg, i);
 *
 * would result in
 *
 *     myreg has type vpiReg
 *     i has type vpiIntegerVar
 */
PLI_INT32 print_vpitype_calltf()
{
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);

    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle arg;
    while((arg = vpi_scan(arg_iterator)) != NULL) {
        PLI_INT32 arg_type = vpi_get(vpiType, arg);
        char* name = vpi_get_str(vpiFullName, arg);
        vpi_printf("%s has type %s\n", name, vpi_get_str(vpiType, arg));
    }

    // because we called vpi_scan(arg_iterator) until it returned null, there's no need to call
    // vpi_free_object() on arg_iterator. Section 27.5 of IEEE 1364 (Verilog-2005's standard)
    // tells us that "The iterator object shall automatically be freed when vpi_scan() returns NULL"

    return 0;
}

PLI_INT32 print_vpitype_compiletf()
{
    // only want to check that systf_handle is valid; number and type of arguments doesn't matter.
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("$print_vpitype: ERROR: NULL systf handle\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    return 0;
}

void print_vpitype_register()
{
    s_vpi_systf_data tfd;

    tfd.type = vpiSysTask;
    tfd.sysfunctype = 0;
    tfd.tfname = "$print_vpitype";
    tfd.calltf = print_vpitype_calltf;
    tfd.compiletf = print_vpitype_compiletf;
    tfd.sizetf = NULL;
    tfd.user_data = NULL;

    vpi_register_systf(&tfd);
}

void (*vlog_startup_routines[])() =
{
    print_vpitype_register,
    0
};
