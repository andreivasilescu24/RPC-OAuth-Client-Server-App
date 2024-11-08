#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database_management.h"

UserDetails **user_details;

char **resources;
int num_users;
int token_valability;
int num_users;
int num_resources;

void load_user_details(char *users_id_file_name, char *resources_file_name, char *approvals_file_name, char *token_valability)
{
    FILE *users_file = fopen(users_id_file_name, "r");
    FILE *resources_file = fopen(resources_file_name, "r");
    FILE *approvals_file = fopen(approvals_file_name, "r");

    token_valability = atoi(token_valability);

    fscanf(users_file, "%d", &num_users);
    fscanf(resources_file, "%d", &num_resources);

    resources = malloc(num_resources * sizeof(char *));
    for (int i = 0; i < num_resources; i++)
    {
        resources[i] = malloc(16 * sizeof(char));
        fscanf(resources_file, "%s", resources[i]);
    }

    user_details = malloc(num_users * sizeof(UserDetails *));

    for (int i = 0; i < num_users; i++)
    {
        UserDetails *current = malloc(sizeof(UserDetails));
        current->user = malloc(16 * sizeof(char));
        fscanf(users_file, "%s", current->user);

        current->num_accesable_resources = 0;
        char *current_line = malloc(100 * sizeof(char));
        fscanf(approvals_file, "%s", current_line);

        if (strcmp(current_line, "*,-") == 0)
        {
            current->resources = NULL;
            current->rights = NULL;
        }
        else
        {
            current->resources = malloc(num_resources * sizeof(char *));
            current->rights = malloc(num_resources * sizeof(char *));

            char *p = strtok(current_line, ",");
            int index = 0;

            while (p)
            {
                current->resources[index] = malloc((strlen(p) + 1) * sizeof(char));
                strcpy(current->resources[index], p);

                p = strtok(NULL, ",");
                current->rights[index] = malloc((strlen(p) + 1) * sizeof(char));
                strcpy(current->rights[index], p);

                p = strtok(NULL, ",");
                index++;
                current->num_accesable_resources++;
            }
        }

        user_details[i] = current;
    }
}

void print_db()
{
    for (int i = 0; i < num_users; i++)
    {
        printf("User: %s\n", user_details[i]->user);
        for (int j = 0; j < user_details[i]->num_accesable_resources; j++)
        {
            printf("Resource: %s, Right: %s\n", user_details[i]->resources[j], user_details[i]->rights[j]);
        }
    }
}