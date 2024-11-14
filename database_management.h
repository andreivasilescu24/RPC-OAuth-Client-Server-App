#include <string>
struct ResourceRights
{
    int read;
    int insert;
    int modify;
    int del;
    int execute;
};

void load_user_details(char *users_file_path, char *resources_file_path, char *approvals_file_path, char *token_valability);
bool user_exists(char *user_id);
std::string get_auth_token(char *id);
void set_user_access_token(char *id, char *access_token);
void set_user_auth_token(char *id, char *auth_token);
// void print_db();
int get_token_valability();
bool user_approves(int request_id);
int update_request_count();
void sign_token(char *token);
bool is_token_signed(char *token);
void set_user_refresh_token(char *id, char *refresh_token);
bool token_exists(char *token);
bool is_token_valid(char *token);
bool resource_exists(char *resource);
bool is_token_op_permitted(char *token, char *operation, char *resource);
void set_access_token_permissions(char *token);
void update_token_op_remaining(char *token);
int get_token_remaining_op(char *token);
bool refresh_token_exists(char *refresh_token);
std::string get_refresh_associated_id(char *refresh_token);
void update_token_perms_on_refresh(char *new_token, char *id);
