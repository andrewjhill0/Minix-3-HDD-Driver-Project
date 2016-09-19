#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include "hello.h"
#include <minix/bdev.h>
#include <minix/u64.h>

/*
 * Function prototypes for the hello driver.
 */
FORWARD _PROTOTYPE( int hello_open,      (message *m) );
FORWARD _PROTOTYPE( int hello_close,     (message *m) );
FORWARD _PROTOTYPE( struct device * hello_prepare, (dev_t device) );
FORWARD _PROTOTYPE( int hello_transfer,  (endpoint_t endpt, int opcode,
                                          u64_t position, iovec_t *iov,
                                          unsigned int nr_req,
                                          endpoint_t user_endpt) );

/* SEF functions and variables. */
FORWARD _PROTOTYPE( void sef_local_startup, (void) );
FORWARD _PROTOTYPE( int sef_cb_init, (int type, sef_init_info_t *info) );
FORWARD _PROTOTYPE( int sef_cb_lu_state_save, (int) );
FORWARD _PROTOTYPE( int lu_state_restore, (void) );



/* Entry points to the hello driver. */
PRIVATE struct chardriver hello_tab =
{
    hello_open,
    hello_close,
    nop_ioctl,
    hello_prepare,
    hello_transfer,
    nop_cleanup,
    nop_alarm,
    nop_cancel,
    NULL
};

/** Represents the /dev/hello device. */
PRIVATE struct device hello_device;

/** State variable to count the number of times the device has been opened. */
PRIVATE int open_counter;

PRIVATE int hello_open(message *UNUSED(m))
{
    dev_t Dev = makedev(3, 0);
    bdev_open(Dev, R_BIT);
    printf("hello_open(). Called %d time(s).\n", ++open_counter);
    return OK;
}

PRIVATE int hello_close(message *UNUSED(m))
{
	dev_t Dev = makedev(3,0);
    bdev_close(Dev);
    printf("hello_close()\n");
    return OK;
}

PRIVATE struct device * hello_prepare(dev_t UNUSED(dev))
{
    hello_device.dv_base = make64(0, 0);
    hello_device.dv_size = make64(strlen(HELLO_MESSAGE), 0);
    return &hello_device;
}

PRIVATE int hello_transfer(endpoint_t endpt, int opcode, u64_t position,
    iovec_t *iov, unsigned nr_req, endpoint_t UNUSED(user_endpt))
{
    int bytes, ret;
    int remaining_size = iov->iov_size;
    int bytes_read = 0;
    int block_size = 1024;
    char buffer[block_size];

    printf("hello_transfer()\n");

    if (nr_req != 1)
    {
        /* This should never trigger for character drivers at the moment. */
        printf("HELLO: vectored transfer request, using first element only\n");
    }

    bytes = strlen(HELLO_MESSAGE) - ex64lo(position) < iov->iov_size ?
            strlen(HELLO_MESSAGE) - ex64lo(position) : iov->iov_size;

    if (bytes <= 0)
    {
        return OK;
    }
    switch (opcode)
    {
        case DEV_GATHER_S:
            //
            //iov->iov_size -= bytes;
		/* This FOR LOOP allows for the handling of reading a block of "block_size" bytes from disk (starting at 0) and then
			copying these back to the user program in a char buffer of the same size.  The loop also makes sure to not
			copy more than it needs to by selecting the minimum of either the remaining bytes to be read and the standard
			copy size of block_size 
		*/
		// NOTE:  At the moment, the block size must be 1024.  Larger block sizes have not been successfully implemented yet
		//    		due to time limitations and will cause a driver crash.

		for(int i = 0; i < iov->iov_size/block_size; i++) 
			{

			if(remaining_size == 0) break;

			if(remaining_size < block_size)	bytes_read = remaining_size;
			else bytes_read = block_size;

			bdev_read(makedev(3, 0), 0, buffer, block_size, 1);

			ret = sys_safecopyto(endpt, (cp_grant_id_t) iov->iov_addr, block_size * i,
                               (vir_bytes) buffer, bytes_read, D		);

			if(remaining_size >= block_size) remaining_size -= block_size;
			else remaining_size = 0;

			}
         break;

        default:
            return EINVAL;
    }
    return ret;
}

PRIVATE int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);

    return OK;
}

PRIVATE int lu_state_restore() {
/* Restore the state. */
    u32_t value;

    ds_retrieve_u32("open_counter", &value);
    ds_delete_u32("open_counter");
    open_counter = (int) value;

    return OK;
}

PRIVATE void sef_local_startup()
{
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

PRIVATE int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
/* Initialize the hello driver. */
    int do_announce_driver = TRUE;

    open_counter = 0;
    switch(type) {
        case SEF_INIT_FRESH:
            printf("%s", HELLO_MESSAGE);
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;

            printf("%sHey, I'm a new version!\n", HELLO_MESSAGE);
        break;

        case SEF_INIT_RESTART:
            printf("%sHey, I've just been restarted!\n", HELLO_MESSAGE);
        break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

PUBLIC int main(void)
{
    /*
     * Perform initialization.
     */
    sef_local_startup();
	dev_t Dev = makedev(3,0);		// create device link to major device #3
	bdev_driver(Dev, "at_wini_0");	// Associate a driver with the given (major) device, using its endpoint
						// File system usage note: typically called from mount and newdriver.
						// This "new driver" aka "mydriver" will utilize at_wini in order to access the 
						// data on the block device (HDD).
    /*
     * Run the main loop.
     */
    chardriver_task(&hello_tab, CHARDRIVER_SYNC);
    return OK;
}

