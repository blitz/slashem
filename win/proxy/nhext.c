/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2001-2002 */
/* NetHack may be freely redistributed.  See license for details. */

#include <stdlib.h>
#include <stdarg.h>
#include "nhxdr.h"
#include "proxycom.h"

/*
 * This module implements the low-level NhExt protocols.
 */

static int no_connections = 0;

/*
 * During development (when we have both the server and the client in one
 * executable) we need to be able to maintain two seperate connections.
 * This will become unnecessary in the final application and the facility
 * should probably be removed for efficiency.
 */

static struct nhext_connection {
    char request[32768];
    char reply[32768];
    int reply_len;
    NhExtXdr *in, *out;
    nhext_io_func read_f, write_f;
    void *read_h, *write_h;
    struct nhext_svc *callbacks;
} nhext_connections[2];

int nhext_subprotocol1_init(nhext_io_func rf, void *rh, nhext_io_func wf, void *wh, struct nhext_svc *cb)
{
    int cn;
    if (no_connections == 2)
	return -1;
    cn = no_connections++;
    nhext_connections[cn].in = (NhExtXdr *)alloc(sizeof(NhExtXdr));
    nhext_connections[cn].out = (NhExtXdr *)alloc(sizeof(NhExtXdr));
    nhext_xdrmem_create(nhext_connections[cn].in,
      nhext_connections[cn].reply, sizeof(nhext_connections[cn].reply),
      NHEXT_XDR_DECODE);
    nhext_xdrmem_create(nhext_connections[cn].out,
      nhext_connections[cn].request, sizeof(nhext_connections[cn].request),
      NHEXT_XDR_ENCODE);
    nhext_connections[cn].read_f = rf;
    nhext_connections[cn].read_h = rh;
    nhext_connections[cn].write_f = wf;
    nhext_connections[cn].write_h = wh;
    nhext_connections[cn].callbacks = cb;
    return cn;
}

void nhext_subprotocol1_end_c(int cn)
{
    nhext_xdr_destroy(nhext_connections[cn].in);
    nhext_xdr_destroy(nhext_connections[cn].out);
    free(nhext_connections[cn].in);
    free(nhext_connections[cn].out);
    nhext_connections[cn].in = NULL;
    nhext_connections[cn].out = NULL;
}

/*
 * This function is available for callers to use if sub-protocol 1 fails.
 * It returns a pointer to the received packet which was being processed
 * at the time.
 */

char *nhext_subprotocol1_get_failed_packet(int cn, int *nb)
{
    *nb = nhext_connections[cn].reply_len;
    return nhext_connections[cn].reply;
}

int nhext_rpc_vparams(NhExtXdr *xdrs, int no, va_list *app)
{
    int retval = TRUE;
    int param;
    va_list ap;
    long param_l;
    long *param_pl;
    int param_i;
    int *param_pi;
    char *param_s;
    char **param_ps, *param_pc;
    nhext_xdr_bool_t param_b, *param_pb;
    int (*param_codec)();
    void *param_addr;
    ap = *app;
    while(retval && no--) {
	param = va_arg(ap, int);
	switch(param) {
	    case EXT_PARAM_INT:
		param_i = va_arg(ap, int);
		retval = nhext_xdr_int(xdrs, &param_i);
		break;
	    case EXT_PARAM_LONG:
		param_l = va_arg(ap, long);
		retval = nhext_xdr_long(xdrs, &param_l);
		break;
	    case EXT_PARAM_STRING:
		param_s = va_arg(ap, char *);
		retval = nhext_xdr_string(xdrs, &param_s, (unsigned int)-1);
		break;
	    case EXT_PARAM_BYTES:
		param_s = va_arg(ap, char *);
		param_i = va_arg(ap, int);
		retval = nhext_xdr_bytes(xdrs, &param_s, &param_i,
			(unsigned int)-1);
		break;
	    case EXT_PARAM_BOOLEAN:
		param_b = va_arg(ap, int);	/* boolean is promoted to int */
		retval = nhext_xdr_bool(xdrs, &param_b);
		break;
	    case EXT_PARAM_CHAR:
		param_i = va_arg(ap, int);	/* char is promoted to int */
		retval = nhext_xdr_int(xdrs, &param_i);
		break;
	    case EXT_PARAM_PTR | EXT_PARAM_INT:
		param_pi = va_arg(ap, int *);
		retval = nhext_xdr_int(xdrs, param_pi);
		break;
	    case EXT_PARAM_PTR | EXT_PARAM_LONG:
		param_pl = va_arg(ap, long *);
		retval = nhext_xdr_long(xdrs, param_pl);
		break;
	    case EXT_PARAM_PTR | EXT_PARAM_STRING:
		param_ps = va_arg(ap, char **);
		*param_ps = NULL;
		retval = nhext_xdr_string(xdrs, param_ps, (unsigned int)-1);
		break;
	    case EXT_PARAM_PTR | EXT_PARAM_BYTES:
		param_ps = va_arg(ap, char **);
		*param_ps = NULL;
		param_pi = va_arg(ap, int *);
		nhext_xdr_bytes(xdrs, param_ps, param_pi, (unsigned int)-1);
		break;
	    case EXT_PARAM_PTR | EXT_PARAM_BOOLEAN:
		param_pb = va_arg(ap, nhext_xdr_bool_t *);
		retval = nhext_xdr_bool(xdrs, param_pb);
		break;
	    case EXT_PARAM_PTR | EXT_PARAM_CHAR:
		param_pc = va_arg(ap, char *);
		retval = nhext_xdr_char(xdrs, param_pc);
		break;
	    case EXT_PARAM_XDRF:
		param_codec = (int (*)())va_arg(ap, void *);
		param_addr = va_arg(ap, void *);
		retval = (*param_codec)(xdrs, param_addr);
		break;
	    default:
		impossible("Bad key in proxy rpc (%d)", param);
		retval = FALSE;
		break;
	}
    }
    *app = ap;
    return retval;
}

int nhext_rpc_params(NhExtXdr *xdrs, int no, ...)
{
    int retval;
    va_list ap;
    va_start(ap, no);
    retval = nhext_rpc_vparams(xdrs, no, &ap);
    if (!retval)
	impossible("NhExt: Codec failed (packet too large?)");
    va_end(ap);
    return retval;
}

/*
 * nhext_rpc() is a generic function to call a remote procedure and return
 * a result. It cannot deal with every possible type of request and result
 * packet, but it's very simple to use for those it can deal with.
 *
 * Arguments are:
 *	id		The ID of the procedure to call
 *	request		List of arguments describing the request packet
 *	response	List of arguments describing the response packet
 *
 * For both the request and response packets, the list of arguments consists
 * of:
 *
 *	no			The number of fields in the packet
 *	field[1] .. field[no]	The field to send/receive.
 */

int
nhext_rpc_c(int cn, unsigned short id, ...)
{
    struct nhext_connection *nc = nhext_connections + cn;
    va_list ap;
    unsigned long value;
    int i, retval;
    int len;		/* Length of the request packet */
    int no;		/* Number of fields */

    nhext_xdr_setpos(nc->out, 4);		/* Leave space for value */
    va_start(ap, id);
    no = va_arg(ap, int);
    if (!nhext_rpc_vparams(nc->out, no, &ap))
	return FALSE;
    len = nhext_xdr_getpos(nc->out) - 4;
    value = (id << 16) | (len >> 2);
    nhext_xdr_setpos(nc->out, 0);
    nhext_xdr_u_long(nc->out, &value);
    if ((*nc->write_f)(nc->write_h, nc->request, len + 4) != len + 4) {
	impossible("Write to proxy interface failed");
	return FALSE;
    }
    do
    {
	retval = nhext_svc_c(cn, nc->callbacks);
	if (retval < 0) {
	    impossible("Proxy server failed");
	    return FALSE;
	}
    } while(retval);
    id = (unsigned short)retval;
    no = va_arg(ap, int);
    if (!nhext_rpc_vparams(nc->in, no, &ap))
	return FALSE;
    if (nhext_xdr_getpos(nc->in) != nc->reply_len) {
	impossible("Unexpected results in proxy rpc (%d, %d)", id, len);
	return FALSE;
    }
    return TRUE;
}

/*
 * nhext_svc() is a function to service incoming packets. It reads a packet
 * from the remote process (either a child or a parent). Service packets
 * (those with non-zero IDs) are dispatched and replied to, reply packets
 * are simply stored in the reply global buffer. The length of the packet
 * received will be stored in the reply_len variable.
 * In both cases, the ID is returned.
 *
 * nhext_svc() is thus suitable for use in two occasions. In the first, it
 * can be used to dispatch callbacks while waiting for a reply. In this mode
 * nhext_svc() should be called repeatedly until it returns 0. In the second
 * case, nhext_svc() can be used as the main loop of a server in which case
 * it should be called repeatedly to service requests and a zero ID should
 * be treated as an error.
 */

int
nhext_svc_c(int cn, struct nhext_svc *services)
{
    struct nhext_connection *nc = nhext_connections + cn;
    int i, len;
    unsigned short id;
    unsigned long value;
    /*
     * FIXME:	This does not cope with fragmentation and relies on
     *		the synchronous nature of the NhExt protocol to prevent
     *		packet merging. This needs to be changed without breaking
     *		the potential for using a seqpacket connection. This
     *		probably means buffering the input.
     */
    nc->reply_len = (*nc->read_f)(nc->read_h, nc->reply, sizeof(nc->reply));
    if (nc->reply_len < 4) {
	if (nc->reply_len > 0)
	    impossible("Bad reply packet (len %d)", nc->reply_len);
	return -1;
    }
    nhext_xdr_setpos(nc->in, 0);
    nhext_xdr_u_long(nc->in, &value);
    id = value >> 16;
    if (nc->reply_len - 4 != (value & 0xffff) << 2) {
	impossible("Bad reply packet (%d != %d)", nc->reply_len - 4,
	  (value & 0xffff) << 2);
	return -1;
    }
    if (id)
    {
	nhext_xdr_setpos(nc->out, 4);
	for(i = 0; services[i].id; i++) {
	    if (id == services[i].id) {
		(*services[i].handler)(id, nc->in, nc->out);
		break;
	    }
	}
	if (!services[i].id)
	    impossible("Unsupported proxy callback ID %d (%d known)", id, i);
	len = nhext_xdr_getpos(nc->out) - 4;
	if (len < 0 || len & 3)
	    impossible("Handler for proxy callback ID %d error", id);
	value = len >> 2;
	nhext_xdr_setpos(nc->out, 0);
	nhext_xdr_u_long(nc->out, &value);
	if ((*nc->write_f)(nc->write_h, nc->request, len + 4) != len + 4) {
	    impossible("Write to proxy interface failed");
	    return -1;
	}
    }
    return id;
}
