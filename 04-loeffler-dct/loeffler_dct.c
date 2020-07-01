#include <stdlib.h>
#include <stdio.h>
#include "vpi_user.h"

// just including the C file cause it's a small project.
#include "dct_88.c"

/**
 * for readability of loeffler_dct_calltf function, we have this struct that groups all of the
 * expected arguments together.
 */
typedef struct loeffler_dct_tf_args
{
    vpiHandle array_in_handle;
    vpiHandle array_out_handle;
} loeffler_dct_tf_args_t;

/**
 * Convenience function that gets all of the args for a loeffler_dct PLI System Task call
 *
 * assumes that there are 4 arguments, and that the systf handle is valid.
 */
loeffler_dct_tf_args_t* loeffler_dct_tf_get_args()
{
    loeffler_dct_tf_args_t* args = calloc(1, sizeof(loeffler_dct_tf_args_t));

    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    args->array_in_handle = vpi_scan(arg_iterator);
    args->array_out_handle = vpi_scan(arg_iterator);

    vpi_free_object(arg_iterator);
    return args;
}

/**
 * Checks the width and height of the given args against array size.
 *
 * These checks need to be done during runtime, not at compiletf time. Returns 0 if the args are good.
 */
int loeffler_dct_tf_check_arg_dims(loeffler_dct_tf_args_t* args)
{
    PLI_INT32 in_size = vpi_get(vpiSize, args->array_in_handle);
    PLI_INT32 out_size = vpi_get(vpiSize, args->array_out_handle);

    if ((in_size != 64) || (out_size != 64)) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * Helper function that pulls every value out of a vpiMemory (an array of registers) and puts it in
 * a newly allocated int8_t array.
 */
int vpi_memory_to_int8_array(int8_t** target, vpiHandle mem)
{
    PLI_INT32 in_size = vpi_get(vpiSize, mem);
    *target = calloc(in_size, sizeof(int8_t));
    vpiHandle reg;
    vpiHandle iter = vpi_iterate(vpiMemoryWord, mem);
    int i = 0;
    while ((reg = vpi_scan(iter)) != NULL) {
        s_vpi_value reg_value;
        reg_value.format = vpiIntVal;
        vpi_get_value(reg, &reg_value);
        (*target)[i++] = (int8_t)reg_value.value.integer;
    }

    return (int)in_size;
}

/**
 * Helper function that takes values out of an int32_t array and puts them in an array of registers.
 *
 * Assumes that the given memory array has at least length len
 */
void int16_array_to_vpi_memory(vpiHandle mem, int16_t* src, int len)
{
    vpiHandle iter = vpi_iterate(vpiMemoryWord, mem);
    for (int i = 0; i < len; i++) {
        vpiHandle reg;
        reg = vpi_scan(iter);

        s_vpi_value reg_value;
        reg_value.format = vpiIntVal;
        reg_value.value.integer = (int32_t)src[i];   // NB: will sign-extend
        vpi_put_value(reg, &reg_value, NULL, vpiNoDelay);
    }
}

/**
 * This PLI task takes an input array and an output array. It's assumed that both the input array
 * and the output array are 8x8 row-major
 *
 * The input array is expected to be 8-bit and to have unsigned pixel values in the range [0, 255].
 * the input array's depth shall be the width of the given image times the length.
 *
 * The output array is expected to be 16-bit signed 7Q8. It will be in row-major order.
 */
PLI_INT32 loeffler_dct_calltf()
{
    // We already know from compiletf that we have 2 args and they're both of the right type
    loeffler_dct_tf_args_t* args = loeffler_dct_tf_get_args();

    // get the width and height values of the image; sanity check them.
    if (loeffler_dct_tf_check_arg_dims(args) != 0) {
        vpi_printf("$loeffler_dct: ERROR: provided width and height are bad.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    // Get the input array values as an int32_t array.
    int8_t* invals;
    vpi_memory_to_int8_array(&invals, args->array_in_handle);

    // do DC subtraction and then take DCT.
    for (int i = 0; i < 64; i++)
        invals[i] -= 128;

    int16_t* outvals = calloc(64, sizeof(int16_t));
    dct88_q8(invals, outvals);

    int16_array_to_vpi_memory(args->array_out_handle, outvals, 64);

    free(invals);
    free(outvals);
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
PLI_INT32 loeffler_dct_compiletf()
{
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("$loeffler_dct: ERROR: NULL systf handle\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    int numargs = count_vpi_args(systf_handle);
    if (numargs != 2) {
        vpi_printf("$loeffler_dct: ERROR: 2 args required; %i args found.\n", numargs);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    loeffler_dct_tf_args_t* args = loeffler_dct_tf_get_args();
    const PLI_INT32 expected_arg_types[] = { vpiMemory, vpiMemory };
    if ((vpi_get(vpiType, args->array_in_handle) != vpiMemory) ||
        (vpi_get(vpiType, args->array_out_handle) != vpiMemory)) {
        vpi_printf("$loeffler_dct: ERROR: all args must have type vpiMemory.\n");
        vpi_printf("%s, %s\n",
                   vpi_get_str(vpiType, args->array_in_handle),
                   vpi_get_str(vpiType, args->array_out_handle));
        vpi_control(vpiFinish, 0);
        return 0;
    }

    // check that the sizes of the first 2 arrays match
    if (loeffler_dct_tf_check_arg_dims(args) != 0) {
        vpi_printf("$loeffler_dct: ERROR: argument arrays must have same size.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    return 0;
}

void loeffler_dct_register()
{
    s_vpi_systf_data tfd;

    tfd.type = vpiSysTask;
    tfd.sysfunctype = 0;
    tfd.tfname = "$loeffler_dct";
    tfd.calltf = loeffler_dct_calltf;
    tfd.compiletf = loeffler_dct_compiletf;
    tfd.sizetf = NULL;
    tfd.user_data = NULL;

    vpi_register_systf(&tfd);
}

void (*vlog_startup_routines[])() =
{
    loeffler_dct_register,
    0
};
