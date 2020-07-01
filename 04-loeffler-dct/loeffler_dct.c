#include <stdlib.h>
#include <stdio.h>
#include "vpi_user.h"

/**
 * Helper function that pulls every value out of a vpiMemory (an array of registers) and puts it in
 * a newly allocated int32_t array.
 */
int vpi_memory_to_int32_array(vpiHandle mem, int32_t** target)
{
    PLI_INT32 in_size = vpi_get(vpiSize, mem);
    *target = calloc(in_size, sizeof(int32_t));

    vpiHandle reg;
    vpiHandle iter = vpi_iterate(vpiMemoryWord, mem);
    int i = 0;
    while ((reg = vpi_scan(iter)) != NULL) {
        s_vpi_value reg_value;
        reg_value.format = vpiIntVal;
        vpi_get_value(reg, &reg_value);
        *target[i++] = reg_value.value.integer;
    }

    return (int)in_size;
}




/**
 * This PLI task takes an input array, an output array, and 2 integer dimensions.
 *
 * The input array is expected to be 8-bit and to have unsigned pixel values in the range [0, 255].
 * the input array's depth shall be the width of the given image times the length.
 *
 * The output array is expected to be 16-bit signed 7Q8. It will be in row-major, MCU-major order.
 * The first 64 values in the output array will correspond to the row-major values in the pixel
 * block starting at (x, y) = (0, 0).
 *
 * The width and height of the image shall both be multiples of 8.
 */
PLI_INT32 loeffler_dct_calltf()
{
    // We already know from compiletf that we have 2 args and they're both of the right type
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle array_in_handle = vpi_scan(arg_iterator);
    vpiHandle array_out_handle = vpi_scan(arg_iterator);
    vpiHandle image_width_handle = vpi_scan(arg_iterator);
    vpiHandle image_height_handle = vpi_scan(arg_iterator);
    //vpi_free_object(arg_iterator);
    vpi_printf("foo\n");

    // check that the sizes of the 2 arrays match. could be in compiletf.
    PLI_INT32 in_size  = vpi_get(vpiSize, array_in_handle);
    PLI_INT32 out_size = vpi_get(vpiSize, array_out_handle);
    vpi_printf("arg a size is %i, arg b size is %i\n", in_size, out_size);
    if (out_size != in_size) {
        vpi_printf("$loeffler_dct: ERROR: argument arrays must have same size.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    // get the width and height values of the image; sanity check them.
    s_vpi_value value_s;
    value_s.format = vpiIntVal;
    vpi_get_value(image_width_handle, &value_s);
    int32_t width = value_s.value.integer;

    value_s.format = vpiIntVal;
    vpi_get_value(image_height_handle, &value_s);
    int32_t height = value_s.value.integer;

    if (((width % 8) != 0) ||
        ((height % 8) != 0) ||
        ((width * height) != in_size)) {
        vpi_printf("$loeffler_dct: ERROR: provided width and height are bad.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }
    int32_t* invals;
    vpi_memory_to_int32_array(array_in_handle, &invals);
    for (int i = 0; i < in_size; i++) {
        printf("%i, ", invals[i]);
    }
    free(invals);

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
    if (numargs != 4) {
        vpi_printf("$loeffler_dct: ERROR: 4 args required; %i args found.\n", numargs);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    const PLI_INT32 expected_arg_types[] = { vpiMemory, vpiMemory, vpiConstant, vpiConstant };
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle arg;
    int i = 0;
    while((arg = vpi_scan(arg_iterator)) != NULL) {
        PLI_INT32 arg_type = vpi_get(vpiType, arg);

        if (arg_type != expected_arg_types[i++]) {
            vpi_printf("$loeffler_dct: ERROR: arg must have type vpiMemory.\n");
            vpi_free_object(arg_iterator);
            vpi_control(vpiFinish, 0);
            return 0;
        }
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
