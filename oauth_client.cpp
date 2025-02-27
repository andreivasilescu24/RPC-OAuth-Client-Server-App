#include "oauth.h"
#include <fstream>
#include <string.h>
#include <iostream>
#include <map>

std::map<std::string, std::string> access_tokens;
std::map<std::string, std::string> refresh_tokens;
std::map<std::string, int> token_ops;

// verifica daca token-ul de acces a expirat si daca este cazul, il actualizeaza
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

		// se reseteaza numarul de operatii ramase pentru noul token
		token_ops[refreshed_tokens->resource_token] = refreshed_tokens->valability;

		delete[] refreshed_tokens->error;
		delete[] refreshed_tokens->resource_token;
		delete[] refreshed_tokens->refresh_token;
		delete[] refresh_token_payload_.refresh_token;
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
			// daca este o operatie de "REQUEST" a treia valoare va fi corespunzatoare refresh-ului automat
			auto_refresh = atoi(p);

			authorization_payload auth_payload;
			auth_payload.id = (char *)malloc((strlen(id) + 1) * sizeof(char));
			strcpy(auth_payload.id, id);
			auth_payload.refresh_token = auto_refresh;

			// cerere token de autorizare
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

				// semnare auth token
				approve_req_token_response *user_approval_response = approve_request_token_1(approval_payload, clnt);

				// cerere token de acces
				access_token_response *access_token_res = request_access_token_1(auth_payload, approval_payload, clnt);

				delete[] approval_payload.auth_token;

				if (strlen(access_token_res->error))
				{
					std::cout << access_token_res->error << std::endl;
				}
				else
				{
					// mapare intre id si token-ul de acces
					access_tokens[id] = access_token_res->resource_token;

					// daca exista un token de refresh pentru id-ul curent, acesta este sters
					if (refresh_tokens.find(id) != refresh_tokens.end())
						refresh_tokens.erase(id);

					// daca exista un token de refresh pentru request-ul curent se va mapa cu id-ul user-ului
					if (strlen(access_token_res->refresh_token))
					{
						refresh_tokens[id] = access_token_res->refresh_token;
						// se seteaza numarul de operatii ramase pentru token-ul de acces
						token_ops[access_tokens[id]] = access_token_res->valability;
						std::cout << auth_res->auth_token << " -> " << access_token_res->resource_token
								  << "," << access_token_res->refresh_token << std::endl;
					}
					else
					{
						std::cout << auth_res->auth_token << " -> " << access_token_res->resource_token << std::endl;
					}
				}

				delete[] access_token_res->error;
				delete[] access_token_res->resource_token;
				delete[] access_token_res->refresh_token;
			}

			delete[] auth_res->auth_token;
			delete[] auth_res->error;
		}
		else
		{
			// daca este o operatie, alta decat "REQUEST" se verifica daca este nevoie de refresh pentru user-ul
			// care face operatia, in cazul in care acesta a optat pentru auto refresh
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
				// caz in care id-ul ar vrea sa faca o operatie fara a avea un token de acces
				delegated_action_payload_.access_token = new char[1];
				strcpy(delegated_action_payload_.access_token, "");
			}

			// validare operatie
			validate_action_response *validate_del_action_response = validate_delegated_action_1(delegated_action_payload_, clnt);
			std::cout << validate_del_action_response->action_output << std::endl;

			delete[] delegated_action_payload_.action;
			delete[] delegated_action_payload_.resource;

			// daca server-ul ne spune ca token-ul a expirat, acesta este sters din storage-ul clientului
			if (!strcmp(validate_del_action_response->action_output, "TOKEN_EXPIRED"))
				access_tokens.erase(id);
			else
			{
				// se decrementeaza numarul de operatii ramase pentru token-ul de acces
				if (token_ops.find(delegated_action_payload_.access_token) != token_ops.end() &&
					strcmp(validate_del_action_response->action_output, "PERMISSION_DENIED"))
					token_ops[delegated_action_payload_.access_token]--;
			}

			delete[] delegated_action_payload_.access_token;
			delete[] validate_del_action_response->action_output;
		}

		delete[] id;
		delete[] operation;
		delete[] resource;
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
