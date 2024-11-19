#include "oauth.h"
#include <fstream>
#include <string.h>
#include <iostream>
#include <map>

std::map<std::string, std::string> access_tokens;
std::map<std::string, std::string> refresh_tokens;
std::map<std::string, int> token_ops;

void check_token_refresh(char *id, CLIENT *clnt)
{
	if (!token_ops[access_tokens[id]])
	{
		refresh_token_payload refresh_token_payload_;
		refresh_token_payload_.refresh_token = new char[16];
		strcpy(refresh_token_payload_.refresh_token, refresh_tokens[id].c_str());
		access_token_response *refreshed_tokens = refresh_token_1(refresh_token_payload_, clnt);

		access_tokens[id] = refreshed_tokens->resource_token;
		refresh_tokens[id] = refreshed_tokens->refresh_token;

		token_ops[refreshed_tokens->resource_token] = refreshed_tokens->valability;
	}
}

void oauth_prog_1(char *host, char *input_file_name)
{
	CLIENT *clnt;

	clnt = clnt_create(host, OAUTH_PROG, OAUTH_VERS, "udp");
	if (clnt == NULL)
	{
		clnt_pcreateerror(host);
		exit(1);
	}

	std::ifstream input_file(input_file_name);

	char line_c[100];
	std::string line;
	while (std::getline(input_file, line))
	{
		strcpy(line_c, line.c_str());
		int auto_refresh = -1;
		char *resource = new char[16];

		char *p = strtok(line_c, ",");
		char *id = new char[strlen(p) + 1];
		strcpy(id, p);

		p = strtok(NULL, ",");
		char *operation = new char[strlen(p) + 1];
		strcpy(operation, p);

		p = strtok(NULL, ",");

		if (!strcmp(operation, "REQUEST"))
		{
			auto_refresh = atoi(p);

			authorization_payload auth_payload;
			auth_payload.id = (char *)malloc((strlen(id) + 1) * sizeof(char));
			strcpy(auth_payload.id, id);
			auth_payload.refresh_token = auto_refresh;

			authorization_response *auth_res = request_authorization_1(auth_payload, clnt);
			if (!strcmp(auth_res->error, "USER_NOT_FOUND"))
			{
				std::cout << auth_res->error << std::endl;
			}
			else
			{
				authz_token_payload approval_payload;
				approval_payload.auth_token = new char[16];
				strcpy(approval_payload.auth_token, auth_res->auth_token);

				approve_req_token_response *user_approval_response = approve_request_token_1(approval_payload, clnt);

				access_token_response *access_token_res = request_access_token_1(auth_payload, approval_payload, clnt);
				if (strlen(access_token_res->error))
				{
					std::cout << access_token_res->error << std::endl;
				}
				else
				{
					access_tokens[id] = access_token_res->resource_token;

					if (refresh_tokens.find(id) != refresh_tokens.end())
						refresh_tokens.erase(id);

					if (strlen(access_token_res->refresh_token))
					{
						refresh_tokens[id] = access_token_res->refresh_token;
						token_ops[access_tokens[id]] = access_token_res->valability;
						std::cout << auth_res->auth_token << " -> " << access_token_res->resource_token
								  << "," << access_token_res->refresh_token << std::endl;
					}
					else
					{
						std::cout << auth_res->auth_token << " -> " << access_token_res->resource_token << std::endl;
					}
				}
			}
		}
		else
		{
			if (refresh_tokens.find(id) != refresh_tokens.end())
				check_token_refresh(id, clnt);

			delegated_action_payload delegated_action_payload_;
			delegated_action_payload_.action = new char[strlen(operation) + 1];
			strcpy(delegated_action_payload_.action, operation);

			delegated_action_payload_.resource = new char[16];
			strcpy(delegated_action_payload_.resource, p);

			if (access_tokens.find(id) != access_tokens.end())
			{
				delegated_action_payload_.access_token = new char[16];
				memcpy(delegated_action_payload_.access_token, access_tokens[id].c_str(), 16);
			}
			else
			{
				delegated_action_payload_.access_token = new char[1];
				strcpy(delegated_action_payload_.access_token, "");
			}

			validate_action_response *validate_del_action_response = validate_delegated_action_1(delegated_action_payload_, clnt);
			std::cout << validate_del_action_response->action_output << std::endl;

			if (!strcmp(validate_del_action_response->action_output, "TOKEN_EXPIRED"))
				access_tokens.erase(id);

			if (!strcmp(validate_del_action_response->action_output, "PERMISSION_DENIED"))
				continue;

			else
			{
				if (token_ops.find(delegated_action_payload_.access_token) != token_ops.end())
					token_ops[delegated_action_payload_.access_token]--;
			}
		}
	}

	input_file.close();
}

int main(int argc, char *argv[])
{
	char *host;

	host = argv[1];
	oauth_prog_1(host, argv[2]);
	exit(0);
}
