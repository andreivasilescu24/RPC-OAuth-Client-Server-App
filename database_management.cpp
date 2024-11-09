#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database_management.h"
#include <map>
#include <string>
#include <fstream>
#include <vector>

// UserDetails **user_details;

// char **resources;
int num_users;
int token_valability;
int num_resources;

std::map<std::string, std::map<std::string, ResourceRights>> user_details;
std::vector<std::string> resources;

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

        std::string line;
        std::getline(approvals_file, line);

        if (line == "*,-")
        {
            user_details[user_id] = std::map<std::string, ResourceRights>();
        }
        else
        {
            std::map<std::string, ResourceRights> user_resources;
            char *line_c = new char[line.size() + 1];
            strcpy(line_c, line.c_str());
            char *p = strtok(line_c, ",");
            while (p)
            {
                char *resource = new char[16];
                strcpy(resource, p);
                p = strtok(NULL, ",");
                user_resources[resource] = get_resource_rights(p);
                p = strtok(NULL, ",");
            }
            user_details[user_id] = user_resources;
        }
    }
}

void print_db()
{

    for (auto user : user_details)
    {
        printf("User: %s\n", user.first.c_str());
        for (auto resource : user.second)
        {
            printf("Resource: %s\n", resource.first.c_str());
            printf("Read: %d\n", resource.second.read);
            printf("Insert: %d\n", resource.second.insert);
            printf("Modify: %d\n", resource.second.modify);
            printf("Delete: %d\n", resource.second.del);
            printf("Execute: %d\n", resource.second.execute);
        }
        printf("\n");
    }
}