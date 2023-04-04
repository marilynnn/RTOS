#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#define THREAD_POOL_PARAM_T dispatch_context_t
#include <sys/iofunc.h>
#include <sys/dispatch.h>


#include "bbs.h"

#include <mutex>
#include <map>

std::mutex mtx;

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;


struct prms{
	std::uint32_t last_el;
	bbs::BBSParams *param;
};
std::map <std::int32_t, prms*> clients;

std::uint32_t parity_bit(uint32_t x) {
	std::uint32_t y = 0;
	while (x > 0) {
		y += x % 2;
		x /= 2;
	};
	return y % 2;
}
std::uint32_t getElem(std::int32_t cl_id) {
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	{
		std::lock_guard<std::mutex> lock(mtx);

		std::uint32_t M = clients[cl_id]->param->p * clients[cl_id]->param->q;
		for (int i = 0; i < sizeof(uint32_t) * 8; ++i) {
			x = clients[cl_id]->last_el * clients[cl_id]->last_el % M;
			clients[cl_id]->last_el = x;

			y = y << 1;
			y = y | parity_bit(x);
		};
	};
	return y;
};
int io_open (resmgr_context_t * ctp , io_open_t * msg , RESMGR_HANDLE_T * handle , void * extra ){
	{
		std::lock_guard<std::mutex> lock(mtx);
		clients[ctp->info.scoid] = new prms();
		clients[ctp->info.scoid]->param = new bbs::BBSParams();
	}
		std::cout << "CLIENT:\t" << ctp->info.scoid << "\tCONNECTED\n";
		return (iofunc_open_default (ctp, msg, handle, extra));
};

int io_close(resmgr_context_t *ctp, io_close_t *msg, iofunc_ocb_t *ocb)
{
	{
		std::lock_guard<std::mutex> lock(mtx);

		std::map <std::int32_t, prms*> :: iterator it;
		it = clients.find(ctp->info.scoid);
		if (clients.count(ctp->info.scoid))
		{
			delete clients[ctp->info.scoid]->param;
			delete clients[ctp->info.scoid];
			clients.erase(it);
					std::cout << "CLIENT:\t" << ctp->info.scoid << "\tCLOSE\n";
				}
				else
					std::cout << "CLIENT = \t" << ctp->info.scoid << " not found" << std::endl;
	}
			return (iofunc_close_dup_default(ctp, msg, ocb));
};

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb) {
	int stat, nbytes;
	if ((stat = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT) {
		return (stat);
	}

	nbytes = 0;
	void* rx_data = _DEVCTL_DATA(msg->i);
	std::uint32_t cl_id = ctp->info.scoid;

	switch (msg->i.dcmd) {

	case _SET_PARAMS: {
		{
		std::lock_guard<std::mutex> lock(mtx);

		bbs::BBSParams* tmp_param = reinterpret_cast<bbs::BBSParams*> (rx_data);
		clients[cl_id]->param->p = tmp_param->p;
		clients[cl_id]->param->q = tmp_param->q;
		clients[cl_id]->param->seed = tmp_param->seed;
		clients[cl_id]->last_el = clients[cl_id]->param->seed;
		}
		break;
	}
	case _GET_ELEM: {
		*(std::uint32_t *) rx_data = getElem(cl_id);
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
    thread_pool_attr_t   pool_attr;
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    thread_pool_t        *tpp;
    dispatch_context_t   *ctp;
    int                  id;

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
	connect_funcs.open 	= io_open;
	io_funcs.close_dup = io_close;

	/* initialize attribute structure used by the device */
	iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

	/* attach our device name */
	id = resmgr_attach(
		dpp, /* dispatch handle  */
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

    /* инициализация атрибутов пула потоков */
    memset(&pool_attr, 0, sizeof pool_attr);
    pool_attr.handle = dpp;
    pool_attr.context_alloc = dispatch_context_alloc;
    pool_attr.block_func = dispatch_block;
    pool_attr.unblock_func = dispatch_unblock;
    pool_attr.handler_func = dispatch_handler;
    pool_attr.context_free = dispatch_context_free;
    pool_attr.lo_water = 2;
    pool_attr.hi_water = 4;
    pool_attr.increment = 1;
    pool_attr.maximum = 50;

    /* инициализация пула потоков */
        if((tpp = thread_pool_create(&pool_attr, POOL_FLAG_EXIT_SELF)) == NULL) {
            fprintf(stderr, "%s: Unable to initialize thread pool.\n",
                    argv[0]);
            return EXIT_FAILURE;
        }

        /* запустить потоки, блокирующая функция */
        thread_pool_start(tpp);
        /* здесь вы не окажетесь, грустно */

	return EXIT_SUCCESS; // never go here
}

