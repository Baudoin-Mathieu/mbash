// SAE S3.03 Systeme mbash IUT CHARLEMAGNE 2025
// BAUDOIN Mathieu
// EYER Nathan

// Pour compiler : gcc mbash.c -o mbash && ./mbash


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>


// Structure representant 1 commande (l'entre d'un utilisateur) ex : ls -l
struct command {   
    char* commande; /* la commande en string */
    int taille;     /* taille de la commande*/
    int curseur;    /* position d'un curseur */
};

// Fonctions sur commande --------------------------------------------------------------------

#define EOF             (-1)
#define ERRCHAR         ( 0)

// Returne le char actuel et avance le curseur de 1
char lire_char(struct command* cmd) {
    cmd->curseur++ ;

    if(cmd->curseur >= cmd->taille) {
        cmd->curseur = cmd->taille;
        return EOF;
    }

    return cmd->commande[cmd->curseur];
}

// Retourne le char actuel sans avancer le curseur
char regarder_char(struct command* cmd) {

    int pos = (cmd->curseur) + 1;

    if(pos >= cmd->taille) return EOF;

    return cmd->commande[pos];
}

// Fin fonctions sur commande --------------------------------------------------------------------------------------------

// Structure representant une commande mise en forme
struct parsed_command {
    char*  func;    
    char* args[200];
    int    nbr_arg;
};


// Prend une commande et retourne une commande mise en forme
struct parsed_command* parse_command(struct command* cmd){

    struct parsed_command* parsed_cmd = malloc(sizeof(struct parsed_command));
    parsed_cmd->nbr_arg = 0 ;

    char* buffer = malloc(1024);
    int buffer_index = -1 ;
    char c ;

    while(regarder_char(cmd)!=EOF){
        c = lire_char(cmd);

        

        switch(c){

            case ' '  :
                if(!parsed_cmd->func){
                    parsed_cmd->func = strdup(buffer) ;
                    parsed_cmd->nbr_arg = parsed_cmd->nbr_arg+1 ;  // premier arg doit etre le nom de la fonction
                    parsed_cmd->args[parsed_cmd->nbr_arg-1] = strdup(buffer) ;   
                }
                else {
                    parsed_cmd->nbr_arg = parsed_cmd->nbr_arg+1 ;
                    parsed_cmd->args[parsed_cmd->nbr_arg-1] = strdup(buffer) ;   
                }
                memset(buffer,0,sizeof(buffer));
                buffer_index = -1 ;
                buffer[buffer_index+1] = '\0';
                break;

            default :
                buffer_index++;
                buffer[buffer_index] = c;
                buffer[buffer_index+1] = '\0';
        }
    }

    if(!parsed_cmd->func){
        parsed_cmd->func = buffer ;
        parsed_cmd->nbr_arg = parsed_cmd->nbr_arg+1 ;
        parsed_cmd->args[parsed_cmd->nbr_arg-1] = strdup(buffer) ; 
    }  
    else {
        parsed_cmd->nbr_arg = parsed_cmd->nbr_arg+1 ;
        parsed_cmd->args[parsed_cmd->nbr_arg-1] = strdup(buffer) ;
    }

    free(buffer);

    parsed_cmd->args[parsed_cmd->nbr_arg] = NULL ; // NULL terminator pour le execv


    return parsed_cmd;
}


// Cherche la path d'un fichier de commande ex : ls, man    A NETTOYER
char* search_path(const char* commandname) {


    char* path_env = getenv("PATH");
    char* dir_start = path_env;  // Pointer to the start of the current directory in PATH
    char* dir_end;  // Pointer to the end of the current directory in PATH

    while (dir_start && *dir_start) {
        dir_end = dir_start;

        // Find the next colon ':' or end of string
        while (*dir_end && *dir_end != ':') {
            dir_end++;
        }

        int dir_length = dir_end - dir_start;  // Length of the current directory path
        if (dir_length == 0) dir_length = 1;  // Ensure the directory length is at least 1

        int filename_length = strlen(commandname);  // Length of the filename

        char full_path[dir_length + 1 + filename_length + 1];  // Allocate space for full path

        // Copy the directory part
        strncpy(full_path, dir_start, dir_length);
        full_path[dir_length] = '\0';

        // Ensure the path ends with a '/'
        if (full_path[dir_length - 1] != '/') {
            strcat(full_path, "/");
        }

        // Append the filename
        strcat(full_path, commandname);

        // Check if the file exists and is a regular file
        struct stat file_stat;
        if (stat(full_path, &file_stat) == 0) {
            if (!S_ISREG(file_stat.st_mode)) {
                errno = ENOENT;
                dir_start = dir_end;
                if (*dir_end == ':') dir_start++;  // Skip the colon to move to the next directory
                continue;
            }

            // Allocate memory for the full path result and return it
            char* result_path = malloc(strlen(full_path) + 1);
            if (!result_path) {
                return NULL;  // Memory allocation failure
            }

            strcpy(result_path, full_path);
            return result_path;
        } else {
            // File not found, move to the next directory in PATH
            dir_start = dir_end;
            if (*dir_end == ':') dir_start++;  // Skip the colon to move to the next directory
        }
    }

    return NULL;  // Fichier introuvable dans le PATH
}



// Prend une commande mise en forme et l'execute
void execute_command(struct parsed_command* parsed_cmd) {

    char* path = search_path(parsed_cmd->func);
    if(!path){
        printf("Fonction introuvable\n");
        return;
    }

    int pid = fork();

    if(pid == 0) {
        execv(path, parsed_cmd->args);
        exit(1);
    }
}


// Prend une commande, la met en forme et l'execute
void parse_and_execute(char* input){

    struct command* cmd = malloc(sizeof(struct command));
    cmd->commande   = strdup(input);
    cmd->taille  = strlen(input)-1;  // Pour enlever le \n
    cmd->curseur   = -1 ;

    struct parsed_command* parsed_cmd = parse_command(cmd);
    free(cmd);
    execute_command(parsed_cmd);
    parsed_cmd->func = NULL ;
    free(parsed_cmd);

}

// MAIN ----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv){

    printf("mBash : SAE Systeme\n");

    char user_input[1024];

    while(true) {
        fprintf(stderr, "$ ");

        fgets(user_input, 1024, stdin) ;

        if(user_input[0] == '\0' || strcmp(user_input, "\n") == 0) continue;

        if(strcmp(user_input, "exit\n") == 0) break;

        if(strncmp(user_input, "cd ", 3) == 0){
            user_input[strlen(user_input)-1] = '\0';
            if(chdir(user_input+3) != 0) perror("cd");
            continue;
        }

        parse_and_execute(user_input);
        memset(user_input,0,sizeof(user_input));
    };
    return 0;
}