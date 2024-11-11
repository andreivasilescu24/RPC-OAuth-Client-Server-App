/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "oauth.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

char **
request_authorization_1(authorization_payload arg1,  CLIENT *clnt)
{
	static char *clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, REQUEST_AUTHORIZATION,
		(xdrproc_t) xdr_authorization_payload, (caddr_t) &arg1,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

access_token_response *
request_access_token_1(authorization_payload arg1, approve_token_payload arg2,  CLIENT *clnt)
{
	request_access_token_1_argument arg;
	static access_token_response clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	arg.arg1 = arg1;
	arg.arg2 = arg2;
	if (clnt_call (clnt, REQUEST_ACCESS_TOKEN, (xdrproc_t) xdr_request_access_token_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_access_token_response, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

void *
validate_delegated_action_1(delegated_action_payload arg1,  CLIENT *clnt)
{
	static char clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, VALIDATE_DELEGATED_ACTION,
		(xdrproc_t) xdr_delegated_action_payload, (caddr_t) &arg1,
		(xdrproc_t) xdr_void, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&clnt_res);
}

approve_req_token_response *
approve_request_token_1(approve_token_payload arg1,  CLIENT *clnt)
{
	static approve_req_token_response clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, APPROVE_REQUEST_TOKEN,
		(xdrproc_t) xdr_approve_token_payload, (caddr_t) &arg1,
		(xdrproc_t) xdr_approve_req_token_response, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}
