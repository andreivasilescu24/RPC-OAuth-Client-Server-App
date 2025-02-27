#include "oauth.h"
#include "database_management.h"
#include "token.h"
#include <iostream>

// intoarce un token de autorizare pentru user-ul cu id-ul primit
authorization_response *
request_authorization_1_svc(authorization_payload arg1, struct svc_req *rqstp)
{
	std::cout << "BEGIN " << arg1.id << " AUTHZ" << std::endl;

	static authorization_response auth_resp;

	// id-ul nu exista in baza de date
	if (!user_exists(arg1.id))
	{
		auth_resp.auth_token = new char[1];
		strcpy(auth_resp.auth_token, "");
		auth_resp.error = new char[15];
		strcpy(auth_resp.error, "USER_NOT_FOUND");
		return &auth_resp;
	}

	auth_resp.auth_token = new char[16];
	auth_resp.error = new char[1];
	strcpy(auth_resp.error, "");
	strcpy(auth_resp.auth_token, generate_access_token(arg1.id));
	set_user_auth_token(arg1.id, auth_resp.auth_token);

	std::cout << "  RequestToken = " << auth_resp.auth_token << std::endl;

	return &auth_resp;
}

// intoarce un token de acces pentru user-ul cu id-ul primit
access_token_response *
request_access_token_1_svc(authorization_payload arg1, authz_token_payload arg2, struct svc_req *rqstp)
{
	static access_token_response result;

	char *auth_token = new char[16];
	strcpy(auth_token, arg2.auth_token);

	// token-ul de autorizare nu este semnat de end-user
	if (!is_token_signed(auth_token))
	{
		result.resource_token = new char[1];
		result.refresh_token = new char[1];
		result.error = new char[15];
		result.valability = 0;

		strcpy(result.error, "REQUEST_DENIED");
		strcpy(result.resource_token, "");
		strcpy(result.refresh_token, "");
		delete[] auth_token;

		return &result;
	}

	result.resource_token = new char[16];
	strcpy(result.resource_token, generate_access_token(auth_token));
	set_user_access_token(arg1.id, result.resource_token);
	set_access_token_permissions(result.resource_token);

	std::cout << "  AccessToken = " << result.resource_token << std::endl;

	// cazul in care user-ul a cerut auto refresh
	if (arg1.refresh_token == 1)
	{
		result.refresh_token = new char[16];
		strcpy(result.refresh_token, generate_access_token(result.resource_token));
		set_user_refresh_token(arg1.id, result.refresh_token);
		std::cout << "  RefreshToken = " << result.refresh_token << std::endl;
	}
	else
	{
		result.refresh_token = new char[1];
		strcpy(result.refresh_token, "");
	}

	result.valability = get_token_valability();
	result.error = new char[1];
	strcpy(result.error, "");

	delete[] auth_token;
	return &result;
}

validate_action_response *validate_delegated_action_1_svc(delegated_action_payload arg1, struct svc_req *rqstp)
{
	static validate_action_response response;

	// token-ul nu exista in baza de date
	if (!token_exists(arg1.access_token))
	{
		response.action_output = new char[18];
		strcpy(response.action_output, "PERMISSION_DENIED");
		std::cout << "DENY (" << arg1.action << "," << arg1.resource << "," << arg1.access_token << "," << get_token_remaining_op(arg1.access_token) << ")" << std::endl;
		return &response;
	}

	// token-ul a expirat
	if (!is_token_valid(arg1.access_token))
	{
		response.action_output = new char[14];
		strcpy(response.action_output, "TOKEN_EXPIRED");
		std::cout << "DENY (" << arg1.action << "," << arg1.resource << ",," << get_token_remaining_op(arg1.access_token) << ")" << std::endl;
		return &response;
	}

	// se updateaza numarul de operatii ramase pentru token-ul actual, deoarece daca se ajunge la linia aceasta
	// inseamna ca token-ul exista si este valid
	update_token_op_remaining(arg1.access_token);

	// resursa nu exista in baza de date
	if (!resource_exists(arg1.resource))
	{
		response.action_output = new char[19];
		strcpy(response.action_output, "RESOURCE_NOT_FOUND");
		std::cout << "DENY (" << arg1.action << "," << arg1.resource << "," << arg1.access_token << "," << get_token_remaining_op(arg1.access_token) << ")" << std::endl;
		return &response;
	}

	// operatia nu este permisa pentru token-ul primit
	if (!is_token_op_permitted(arg1.access_token, arg1.action, arg1.resource))
	{
		response.action_output = new char[24];
		strcpy(response.action_output, "OPERATION_NOT_PERMITTED");
		std::cout << "DENY (" << arg1.action << "," << arg1.resource << "," << arg1.access_token << "," << get_token_remaining_op(arg1.access_token) << ")" << std::endl;
		return &response;
	}

	response.action_output = new char[19];
	strcpy(response.action_output, "PERMISSION_GRANTED");
	std::cout << "PERMIT (" << arg1.action << "," << arg1.resource << "," << arg1.access_token << "," << get_token_remaining_op(arg1.access_token) << ")" << std::endl;
	return &response;
}

// intoarce token-ul de autorizare, impreuna cu un flag "is_signed" care indica daca user-ul a semnat
// sau nu token-ul de autorizare
approve_req_token_response *approve_request_token_1_svc(authz_token_payload arg1, struct svc_req *rqstp)
{

	static approve_req_token_response result;

	// obtinem id-ul cererii de request actuale pentru a putea obtine permisiunile acordate
	int curr_req_id = update_request_count();

	result.auth_token = arg1.auth_token;

	// end-user semneaza token-ul de autorizare
	if (user_approves(curr_req_id))
	{
		result.is_signed = 1;
		sign_token(result.auth_token);
	}
	else
	{
		result.is_signed = 0;
	}

	return &result;
}

// utilizat pentru a separa logica de refresh token de cea de generate access token
access_token_response *refresh_token_1_svc(refresh_token_payload argp, struct svc_req *rqstp)
{
	static access_token_response result;

	// id-ul user-ului asociat cu refresh token-ul primit
	std::string id = get_refresh_associated_id(argp.refresh_token);
	char *id_c = new char[id.length() + 1];
	strcpy(id_c, id.c_str());

	std::cout << "BEGIN " << id << " AUTHZ" << " REFRESH" << std::endl;

	// noul token de acces pentru user
	result.resource_token = new char[16];
	strcpy(result.resource_token, generate_access_token(argp.refresh_token));

	// se updateaza permisiunile pentru noul access token, aceleasi cu cele
	// ale vechiului token
	update_token_perms_on_refresh(result.resource_token, id_c);

	// se seteaza noul token de acces pentru user
	set_user_access_token(id_c, result.resource_token);

	std::cout << "  AccessToken = " << result.resource_token << std::endl;

	// noul token de refresh pentru user
	result.refresh_token = new char[16];
	strcpy(result.refresh_token, generate_access_token(result.resource_token));

	// se seteaza noul token de refresh pentru user
	set_user_refresh_token(id_c, result.refresh_token);
	std::cout << "  RefreshToken = " << result.refresh_token << std::endl;

	result.valability = get_token_valability();
	result.error = new char[1];
	strcpy(result.error, "");

	return &result;
}