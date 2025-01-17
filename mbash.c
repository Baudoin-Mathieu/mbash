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
        parsed_cmd->func = strdup(buffer) ;
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

    if (pid == 0) {
        execv(path, parsed_cmd->args);
        perror("Erreur execv"); // En cas d'échec
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Attendre la fin du processus enfant
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("Erreur fork");
    }
}


// Prend une commande, la met en forme et l'execute
void parse_and_execute(char* input){

    struct command* cmd = malloc(sizeof(struct command));
    cmd->commande   = strdup(input);
    cmd->taille  = strlen(input);  // Pour enlever le \n
    cmd->curseur   = -1 ;

    struct parsed_command* parsed_cmd = parse_command(cmd);
    free(cmd);
    execute_command(parsed_cmd);
    parsed_cmd->func = NULL ;
    free(parsed_cmd);

}


//HISTORIQUE (fleches) ---------------------------------------------------------------------------------------------------------------------
#include <termios.h>
#define MAX_HISTORIQUE 100 //Taille max de l'historique

//Structure pour l'historique
char *historique[MAX_HISTORIQUE];
int nb_historique = 0;
int index_historique = -1;

//Ajoute une commande à l'historique
void ajouter_historique(const char *cmd) {
    if (nb_historique == MAX_HISTORIQUE) {
        free(historique[0]); //Supprime le plus vieux
        memmove(historique, historique + 1, (MAX_HISTORIQUE - 1) * sizeof(char *));
        nb_historique--;
    }
    historique[nb_historique++] = strdup(cmd);
    if (!historique[nb_historique - 1]) {
        perror("Erreur d'allocation mémoire");
        exit(EXIT_FAILURE);
    }

}

//Permet de detecter la pression des touches
void detecter_touche(struct termios *termios) {
    struct termios raw = *termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

//Restaure le terminal
void pasdetecter_touche(struct termios *termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, termios);
}

// Trouve les fichiers et dossiers correspondant au préfixe donné
#include <dirent.h>
#include <ctype.h>
void auto_completion(char *input, size_t *pos) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if (dir == NULL) {
        perror("Impossible d'ouvrir le répertoire");
        return;
    }

    char matches[100][256]; // Liste des correspondances
    int match_count = 0;

    // Parcourt les fichiers du répertoire
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (strncmp(entry->d_name, input, strlen(input)) == 0) {
            strcpy(matches[match_count++], entry->d_name);
        }
    }
    closedir(dir);

    if (match_count == 1) {
        // Une seule correspondance, compléter automatiquement
        snprintf(input, 1024, "%s", matches[0]);
        *pos = strlen(input);
        printf("\r\33[2K$ %s", input);
        fflush(stdout);
    } else if (match_count > 1) {
        // Plusieurs correspondances, afficher les options
        printf("\n");
        for (int i = 0; i < match_count; i++) {
            printf("%s  ", matches[i]);
        }
        printf("\n$ %s", input);
        fflush(stdout);
    }
}


//Permet la gestion des fleches haut et bas
bool lire(char *cmd, size_t size) {
    struct termios termios;
    tcgetattr(STDIN_FILENO, &termios);
    detecter_touche(&termios);

    size_t pos = 0;
    int c;

    while (true) {
        c = getchar();
        if (c == '\n') { //Touche Entree
            cmd[pos] = '\0';
            printf("\n");
            break;
        } else if (c == 127) { //Touche effacer
            if (pos > 0) {
                pos--;
                printf("\b \b"); //Supprime le dernier caractere affiche
            }
        } else if (c == '\033') { //Fleches
            getchar();           //Ignore les '['
            switch (getchar()) { //Fleche directionnelle
                case 'A': //Fleche haut
                    if (nb_historique > 0 && index_historique > 0) {
                        index_historique--;
                        pos = snprintf(cmd, size, "%s", historique[index_historique]);
                        printf("\r\33[2K$ %s", cmd); //Reecrit la ligne
                        fflush(stdout);
                    }
                    break;
                case 'B': //Fleche bas
                    if (nb_historique > 0 && index_historique < nb_historique - 1) {
                        index_historique++;
                        pos = snprintf(cmd, size, "%s", historique[index_historique]);
                        printf("\r\33[2K$ %s", cmd); //Reecrit la ligne
                        fflush(stdout);
                    } else if (index_historique == nb_historique - 1) {
                        index_historique = nb_historique;
                        pos = 0;
                        cmd[0] = '\0';
                        printf("\r\33[2K$ "); //Vide la ligne
                    }
                    break;
            }
        }else if(c == '\t'){
            cmd[pos] = '\0';
            auto_completion(cmd, &pos);
        }else if (pos < size - 1) { //Autres caracteres
            cmd[pos++] = c;
            printf("%c", c);
        }
    }

    pasdetecter_touche(&termios); //Retour a l'etat de base
    return pos > 0;
}



// MAIN ----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv){

    printf("mBash : SAE Systeme\n");

    char user_input[1024];

    while(true) {
        printf("$ ");
        fflush(stdout);
        index_historique = nb_historique;

        if (!lire(user_input, sizeof(user_input))) {
                printf("\n");  // Retour à la ligne propre si la commande est vide
                continue;
        }

        if(user_input[0] == '\0' || strcmp(user_input, "\n") == 0) continue;

        if(strcmp(user_input, "exit") == 0) break;

        if (strncmp(user_input, "cd ", 3) == 0) {
            // Supprimer le caractère de fin de ligne '\n' s'il est présent
            size_t len = strlen(user_input);
            if (user_input[len - 1] == '\n') {
                user_input[len - 1] = '\0';
            }

            // Extraire le chemin et essayer de changer de répertoire
            const char *path = user_input + 3; // Ignorer "cd "
            if (chdir(path) != 0) {
                perror("cd"); // Affiche l'erreur si chdir échoue
            }
            continue;
        }


        if(strcmp(user_input, "history") == 0){
            for(int i=0;i<nb_historique;i++){
                printf("%d: %s\n", i+1, historique[i]);
            }
            ajouter_historique(user_input);
            continue;
        }

        ajouter_historique(user_input);

        parse_and_execute(user_input);
        memset(user_input,0,sizeof(user_input));
    };

    return 0;
}