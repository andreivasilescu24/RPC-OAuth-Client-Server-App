struct authorization_payload {
    string id <16>;
    int refresh_token;
};

struct authorization_response {
    string auth_token <16>;
    string error <>;
};

struct authz_token_payload {
    string auth_token <16>;
};

struct access_token_response {
    string resource_token <16>;
    string refresh_token <16>;
    string error <>;
    int valability;
};

struct delegated_action_payload {
    string action <>;
    string resource<>;
    string access_token<16>;
};

struct validate_action_response {
    string action_output <>;
};

struct approve_req_token_response{
    string auth_token <16>;
    int is_signed;
};

struct refresh_token_payload {
    string refresh_token <16>;
};

program OAUTH_PROG {
    version OAUTH_VERS {
        authorization_response REQUEST_AUTHORIZATION(authorization_payload) = 1;
        access_token_response REQUEST_ACCESS_TOKEN(authorization_payload, authz_token_payload) = 2;
        validate_action_response VALIDATE_DELEGATED_ACTION(delegated_action_payload) = 3;
        approve_req_token_response APPROVE_REQUEST_TOKEN(authz_token_payload) = 4;
        access_token_response REFRESH_TOKEN(refresh_token_payload) = 5;
    } = 1;
} = 0x12345678;