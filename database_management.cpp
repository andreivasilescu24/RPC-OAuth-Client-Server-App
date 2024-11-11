#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database_management.h"
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

int token_valability;
int num_users;
int num_resources;
int request_count = 1;

std::vector<std::string> resources;
std::vector<std::string> users;
std::map<int, std::map<std::string, ResourceRights>> request_approvals;
std::map<std::string, std::string> auth_tokens;
std::map<std::string, std::string> access_tokens;
std::map<std::string, int> token_approvals;
std::map<std::string, std::string> refresh_tokens;
std::map<std::string, int> signed_tokens;

ResourceRights get_resource_rights(char *rights)
{
    int read = strchr(rights, 'R') ? 1 : 0;
    int insert = strchr(rights, 'I') ? 1 : 0;
    int modify = strchr(rights, 'M') ? 1 : 0;
    int del = strchr(rights, 'D') ? 1 : 0;
    int execute = strchr(rights, 'X') ? 1 : 0;

    ResourceRights rights_struct;
    rights_struct.read = read;
    rights_struct.insert = insert;
    rights_struct.modify = modify;
    rights_struct.del = del;
    rights_struct.execute = execute;
    return rights_struct;
}

std::string get_auth_token(char *id)
{
    return auth_tokens[id];
}

void set_user_access_token(char *id, char *access_token)
{
    access_tokens[id] = access_token;
}

void set_user_auth_token(char *id, char *auth_token)
{
    auth_tokens[id] = auth_token;
}

int get_token_valability()
{
    return token_valability;
}

bool user_approves(int request_id)
{
    if (request_approvals.find(request_id) == request_approvals.end())
        return false;
    return true;
}

void sign_token(char *token)
{
    signed_tokens[token] = 1;
}

bool is_token_signed(char *token)
{
    return signed_tokens.find(token) != signed_tokens.end();
}

int update_request_count()
{
    int curr_req_id = request_count;
    request_count++;
    return curr_req_id;
}

void load_user_details(char *users_id_file_name, char *resources_file_name, char *approvals_file_name, char *token_valability_arg)
{
    std::ifstream users_file(users_id_file_name);
    std::ifstream resources_file(resources_file_name);
    std::ifstream approvals_file(approvals_file_name);

    token_valability = atoi(token_valability_arg);

    users_file >> num_users;
    resources_file >> num_resources;

    resources.resize(num_resources);
    for (int i = 0; i < num_resources; i++)
    {
        std::getline(resources_file, resources[i]);
    }

    for (int i = 0; i < num_users; i++)
    {
        char *user_id = new char[16];
        users_file >> user_id;

        users.push_back(user_id);
    }

    std::string line;
    int i = 1;
    while (std::getline(approvals_file, line))
    {
        if (line == "*,-")
        {
            i++;
            continue;
        }
        char *line_c = new char[line.size() + 1];
        strcpy(line_c, line.c_str());
        char *p = strtok(line_c, ",");
        while (p)
        {
            char *resource = new char[16];
            strcpy(resource, p);
            p = strtok(NULL, ",");
            request_approvals[i][resource] = get_resource_rights(p);
            p = strtok(NULL, ",");
        }
        i++;
    }
}

bool user_exists(char *user_id)
{
    return std::find(users.begin(), users.end(), user_id) != users.end();
}

void print_db()
{
    printf("Users:\n");
    for (auto user : users)
    {
        printf("%s\n", user.c_str());
    }

    printf("Resources:\n");
    for (auto resource : resources)
    {
        printf("%s\n", resource.c_str());
    }

    printf("Approvals:\n");
    for (auto approval : request_approvals)
    {
        printf("Request %d:\n", approval.first);
        for (auto resource : approval.second)
        {
            printf("\tResource: %s\n", resource.first.c_str());
            printf("\t\tRead: %d\n", resource.second.read);
            printf("\t\tInsert: %d\n", resource.second.insert);
            printf("\t\tModify: %d\n", resource.second.modify);
            printf("\t\tDelete: %d\n", resource.second.del);
            printf("\t\tExecute: %d\n", resource.second.execute);
        }
    }

    printf("Auth Tokens:\n");
    for (auto token : auth_tokens)
    {
        printf("User: %s\n", token.first.c_str());
        printf("\tToken: %s\n", token.second.c_str());
    }

    printf("Access Tokens:\n");
    for (auto token : access_tokens)
    {
        printf("User: %s\n", token.first.c_str());
        printf("\tToken: %s\n", token.second.c_str());
    }

    printf("Token Approvals:\n");
    for (auto token : token_approvals)
    {
        printf("Token: %s\n", token.first.c_str());
        printf("\tApprovals: %d\n", token.second);
    }

    printf("Refresh Tokens:\n");
    for (auto token : refresh_tokens)
    {
        printf("Token: %s\n", token.first.c_str());
        printf("\tRefresh Token: %s\n", token.second.c_str());
    }

    printf("Signed Tokens:\n");
    for (auto token : signed_tokens)
    {
        printf("Token: %s\n", token.first.c_str());
    }
}