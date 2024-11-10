/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "oauth.h"
#include "database_management.h"
#include "token.h"
#include <fstream>

char **
request_authorization_1_svc(authorization_payload arg1, struct svc_req *rqstp)
{
	std::ofstream output_file;
	output_file.open("tests_output/test1/server.out", std::ios_base::app);
	output_file << "BEGIN " << arg1.id << " AUTHZ\n";

	static char *result = new char[16];

	if (!user_exists(arg1.id))
	{
		strcpy(result, "USER_NOT_FOUND");
		output_file.close();
		return &result;
	}

	strcpy(result, generate_access_token(arg1.id));

	output_file << "\tRequestToken = " << result << "\n";
	output_file.close();

	return &result;
}

access_token_response *
request_access_token_1_svc(authorization_payload arg1, access_token_payload arg2, struct svc_req *rqstp)
{

	std::ofstream output_file;
	output_file.open("tests_output/test1/server.out", std::ios_base::app);
	static access_token_response result;

	const char *auth_token_c = get_auth_token(arg1.id).c_str();
	char *auth_token = new char[16];
	strcpy(auth_token, auth_token_c);

	result.resource_token = generate_access_token(auth_token);
	set_user_access_token(arg1.id, result.resource_token);
	result.refresh_token = generate_access_token(result.resource_token);
	result.valability = get_token_valability();

	output_file << "\tAccessToken = " << result.resource_token << "\n";
	output_file.close();

	return &result;
}

void *validate_delegated_action_1_svc(delegated_action_payload arg1, struct svc_req *rqstp)
{
	return NULL;
}
