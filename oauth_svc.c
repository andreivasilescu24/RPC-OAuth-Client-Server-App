/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "oauth.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "database_management.c"

#ifndef SIG_PF
#define SIG_PF void (*)(int)
#endif

static char **
_request_authorization_1(authorization_payload *argp, struct svc_req *rqstp)
{
	return (request_authorization_1_svc(*argp, rqstp));
}

static access_token_response *
_request_access_token_1(request_access_token_1_argument *argp, struct svc_req *rqstp)
{
	return (request_access_token_1_svc(argp->arg1, argp->arg2, rqstp));
}

static void
oauth_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union
	{
		authorization_payload request_authorization_1_arg;
		request_access_token_1_argument request_access_token_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc)
	{
	case NULLPROC:
		(void)svc_sendreply(transp, (xdrproc_t)xdr_void, (char *)NULL);
		return;

	case REQUEST_AUTHORIZATION:
		_xdr_argument = (xdrproc_t)xdr_authorization_payload;
		_xdr_result = (xdrproc_t)xdr_wrapstring;
		local = (char *(*)(char *, struct svc_req *))_request_authorization_1;
		break;

	case REQUEST_ACCESS_TOKEN:
		_xdr_argument = (xdrproc_t)xdr_request_access_token_1_argument;
		_xdr_result = (xdrproc_t)xdr_access_token_response;
		local = (char *(*)(char *, struct svc_req *))_request_access_token_1;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	memset((char *)&argument, 0, sizeof(argument));
	if (!svc_getargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument))
	{
		svcerr_decode(transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t)_xdr_result, result))
	{
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument))
	{
		fprintf(stderr, "%s", "unable to free arguments");
		exit(1);
	}
	return;
}

int main(int argc, char **argv)
{
	register SVCXPRT *transp;

	load_user_details(argv[1], argv[2], argv[3], argv[4]);

	pmap_unset(OAUTH_PROG, OAUTH_VERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL)
	{
		fprintf(stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, OAUTH_PROG, OAUTH_VERS, oauth_prog_1, IPPROTO_UDP))
	{
		fprintf(stderr, "%s", "unable to register (OAUTH_PROG, OAUTH_VERS, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL)
	{
		fprintf(stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, OAUTH_PROG, OAUTH_VERS, oauth_prog_1, IPPROTO_TCP))
	{
		fprintf(stderr, "%s", "unable to register (OAUTH_PROG, OAUTH_VERS, tcp).");
		exit(1);
	}

	svc_run();

	fprintf(stderr, "%s", "svc_run returned");
	exit(1);
	/* NOTREACHED */
}
