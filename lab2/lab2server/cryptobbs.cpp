#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "bbs.h"

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

std::uint32_t last_el;
bbs::BBSParams *param;

std::uint32_t parity_bit(uint32_t x) {
	std::uint32_t y = 0;
	while (x > 0) {
		y += x % 2;
		x /= 2;
	};
	return y % 2;
}
std::uint32_t getElem() {
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	std::uint32_t M = param->p * param->q;

	for (int i = 0; i < sizeof(uint32_t) * 8; ++i) {
		x = last_el * last_el % M;
		last_el = x;

		y = y << 1;
		y = y | parity_bit(x);
	};

	return y;
}
;

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb) {
	int stat, nbytes;
	if ((stat = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT) {
		return (stat);
	}

	stat = nbytes = 0;
	void* rx_data = _DEVCTL_DATA(msg->i);

	switch (msg->i.dcmd) {
	case _SET_PARAMS: {
		param = reinterpret_cast<bbs::BBSParams *>(rx_data);
		last_el = param->seed;
		break;
	}
	case _GET_ELEM: {
		*(std::uint32_t *) rx_data = getElem();
		nbytes = sizeof(std::uint32_t);
		break;
	}
	default:
		return (ENOSYS);
	};
	memset(&(msg->o), 0, sizeof(msg->o));
	msg->o.nbytes = nbytes;
	return (_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));
}
;

int main(int argc, char **argv) {
	/* declare variables we'll be using */
	resmgr_attr_t resmgr_attr;
	dispatch_t *dpp;
	dispatch_context_t *ctp;
	int id;

	/* initialize dispatch interface */
	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr,
		"%s: Unable to allocate dispatch handle.\n",
		argv[0]);
		return EXIT_FAILURE;
	}

		/* initialize resource manager attributes */
	memset(&resmgr_attr, 0, sizeof resmgr_attr);
	resmgr_attr.nparts_max = 1;
	resmgr_attr.msg_max_size = 2048;

	/* initialize functions for handling messages */
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);

	io_funcs.devctl = io_devctl;

	/* initialize attribute structure used by the device */
	iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

	/* attach our device name */
	id = resmgr_attach(dpp, /* dispatch handle        */
	&resmgr_attr, /* resource manager attrs */
	"/dev/cryptobbs", /* device name         */
	_FTYPE_ANY, /* open type              */
	0, /* flags                  */
	&connect_funcs, /* connect routines       */
	&io_funcs, /* I/O routines           */
	&attr); /* handle                 */

	if (id == -1) {
		fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
		return EXIT_FAILURE;
	}

		/* allocate a context structure */
	ctp = dispatch_context_alloc(dpp);

	/* start the resource manager message loop */
	while (1) {
		if ((ctp = dispatch_block(ctp)) == NULL) {
			fprintf(stderr, "block error\n");
			return EXIT_FAILURE;
		}
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS; // never go here
}

