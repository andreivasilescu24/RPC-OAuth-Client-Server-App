#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database_management.h"
#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>

int token_valability;
int num_users;
int num_resources;
int request_count = 1; // id-ul cererii de request curente

std::vector<std::string> resources;                                             // resursele de pe server
std::vector<std::string> users;                                                 // userii de pe server
std::map<int, std::map<std::string, ResourceRights>> request_approvals;         // mapare intre id-ul unui request si drepturile asociate
std::map<std::string, std::string> auth_tokens;                                 // token-urile de autorizare ale userilor
std::map<std::string, std::string> access_tokens;                               // token-urile de acces ale userilor
std::map<std::string, std::map<std::string, ResourceRights>> token_permissions; // permisiunile asociate token-urilor de acces
std::map<std::string, std::string> refresh_tokens;                              // token-urile de refresh ale userilor
std::map<std::string, int> signed_tokens;                                       // token-urile de autorizare semnate
std::map<std::string, int> token_operations_remaining;                          // numarul de operatii ramase pentru un token de acces

// intoarce structura ce contine drepturile primite sub forma de string, asa cum sunt citite din fisierul de input
// fiecare tip de drept va avea valoarea de 0 sau 1 in functie de existenta acestuia in string
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

// seteaza token-ul de acces pentru un user in baza de date
void set_user_access_token(char *id, char *access_token)
{
    access_tokens[id] = access_token;
    token_operations_remaining[access_token] = token_valability;
}

// seteaza token-ul de refresh pentru un user in baza de date
void set_user_refresh_token(char *id, char *refresh_token)
{
    // se sterge vechiul token de refresh asociat user-ului
    for (auto entry : refresh_tokens)
        if (entry.second == id)
        {
            refresh_tokens.erase(entry.first);
            break;
        }

    refresh_tokens[refresh_token] = id;
}

// seteaza token-ul de autorizare pentru un user in baza de date
void set_user_auth_token(char *id, char *auth_token)
{
    auth_tokens[id] = auth_token;
}

int get_token_valability()
{
    return token_valability;
}

// verifica daca user-ul a aprobat cererea de request
bool user_approves(int request_id)
{
    if (request_approvals.find(request_id) == request_approvals.end())
        return false;
    return true;
}

// semneaza un token de autorizare
void sign_token(char *token)
{
    signed_tokens[token] = 1;
}

bool is_token_signed(char *token)
{
    return signed_tokens.find(token) != signed_tokens.end();
}

// intoarce id-ul cererii de request curente si incrementeaza
// valoarea in baza de date a server-ului
int update_request_count()
{
    int curr_req_id = request_count;
    request_count++;
    return curr_req_id;
}

// verifica daca token-ul de acces exista in baza de date
bool token_exists(char *token)
{
    std::string token_str = token;
    for (auto entry : access_tokens)
        if (entry.second == token_str)
            return true;

    return false;
}

// verifica daca token-ul de acces mai are operatii ramase
bool is_token_valid(char *token)
{
    if (token_operations_remaining[token])
        return true;
    return false;
}

// verifica daca o resursa exista in baza de date
bool resource_exists(char *resource)
{
    for (auto res : resources)
        if (res == std::string(resource))
            return true;

    return false;
}

// verifica daca o operatie este permisa pentru un token de acces
bool is_token_op_permitted(char *token, char *operation, char *resource)
{
    std::string operation_str = operation;
    int is_op_permitted = 0;

    // daca token-ul nu are permisiuni asociate resursei care doreste sa fie accesata, operatia nu este permisa
    if (token_permissions[token].find(resource) == token_permissions[token].end())
        return is_op_permitted;

    if (operation_str == "READ")
    {
        is_op_permitted = token_permissions[token][resource].read;
    }
    else if (operation_str == "INSERT")
    {
        is_op_permitted = token_permissions[token][resource].insert;
    }
    else if (operation_str == "MODIFY")
    {
        is_op_permitted = token_permissions[token][resource].modify;
    }
    else if (operation_str == "DELETE")
    {
        is_op_permitted = token_permissions[token][resource].del;
    }
    else if (operation_str == "EXECUTE")
    {
        is_op_permitted = token_permissions[token][resource].execute;
    }

    return is_op_permitted;
}

// seteaza permisiunile unui token de acces bazat pe request id-ul la care permisiunile sunt mapate
void set_access_token_permissions(char *token)
{
    // se iau permsiunile de la request_count - 1 pentru ca request_count este incrementat inainte de a se apela aceasta functie,
    // atunci cand user-ul semneaza auth token-ul
    token_permissions[token] = request_approvals[request_count - 1];
}

// decrementeaza numarul de operatii ramase pentru un token de acces
void update_token_op_remaining(char *token)
{
    if (token_operations_remaining.find(token) != token_operations_remaining.end())
        token_operations_remaining[token]--;
}

// intoarce numarul de operatii ramase pentru un token de acces
int get_token_remaining_op(char *token)
{
    if (token_operations_remaining.find(token) != token_operations_remaining.end())
        return token_operations_remaining[token];
    return 0;
}

// verifica daca un token de refresh exista in baza de date
bool refresh_token_exists(char *refresh_token)
{
    return refresh_tokens.find(refresh_token) != refresh_tokens.end();
}

// intoarce id-ul user-ului asociat cu un token de refresh
std::string get_refresh_associated_id(char *refresh_token)
{
    return refresh_tokens[refresh_token];
}

// actualizeaza permisiunile unui token de acces dupa ce acesta a fost refreshed
void update_token_perms_on_refresh(char *new_token, char *id)
{
    // se copiaza permisiunile token-ului vechi in cel nou
    std::string ex_access_token = access_tokens[id];
    token_permissions[new_token] = token_permissions[ex_access_token];

    // se sterg permisiunile vechiului token din baza de date
    token_permissions.erase(ex_access_token);
}

// verifica daca un user exista in baza de date
bool user_exists(char *user_id)
{
    return std::find(users.begin(), users.end(), user_id) != users.end();
}

// incarca datele despre useri, resurse si drepturile asociate din fisierele de input, creand baza de date a server-ului
// aceasta functie este apelata la inceperea rularii server-ului
void load_user_details(char *users_id_file_name, char *resources_file_name,
                       char *approvals_file_name, char *token_valability_arg)
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
        resources_file >> resources[i];
    }

    for (int i = 0; i < num_users; i++)
    {
        char *user_id = new char[16];
        users_file >> user_id;

        users.push_back(user_id);
        delete[] user_id;
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
            delete[] resource;
        }
        delete[] line_c;
        i++;
    }
}
