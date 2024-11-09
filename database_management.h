#include <map>
struct ResourceRights
{
    int read;
    int insert;
    int modify;
    int del;
    int execute;
};

void load_user_details(char *users_file_path, char *resources_file_path, char *approvals_file_path, char *token_valability);
void print_db();
