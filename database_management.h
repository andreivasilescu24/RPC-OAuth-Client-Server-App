typedef struct
{
    char *user;
    char **resources;
    char **rights;
    int num_accesable_resources;
} UserDetails;

void load_user_details(char *users_file_path, char *resources_file_path, char *approvals_file_path, char *token_valability);
UserDetails *get_user_details(char *username);
void print_db();
