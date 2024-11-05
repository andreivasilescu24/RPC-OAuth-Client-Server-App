struct authorization_payload {
    string id <16>;
};

struct access_token_payload {
    string token <16>;
};

struct access_token_response {
    string resource_token <16>;
    string refresh_token <16>;
    int valability;
};

program OAUTH_PROG {
    version OAUTH_VERS {
        string REQUEST_AUTHORIZATION(authorization_payload) = 1;
        access_token_response REQUEST_ACCESS_TOKEN(authorization_payload, access_token_payload) = 2;
    } = 1;
} = 0x12345678;