// SAE S3 Système mbash IUT CHARLEMAGNE 2025
// BAUDOIN Mathieu
// EYER Nathan

// Pour compiler : gcc mbash.c -o mbash
// Pour executer : ./mbash

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
    char cmd[1024];

    while(1) {
        fprintf(stderr, "$ ");

        fgets(cmd, sizeof(cmd), stdin);

        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0) {
            continue;
        }

        if(strcmp(cmd, "exit\n") == 0) {
            break;
        }

        printf("%s", cmd);
    };

    exit(EXIT_SUCCESS);
}

struct node_s *parse_command(struct token_s *token){
    if(!token) return NULL; //Verifie qu'il y a bien un parametre

    struct node_s *cmd = new_node(NODE_COMMAND);
    if(!cmd) return NULL; //Verifie qu'il y a bien un cmd

    struct source_s *src = token->src; //Assure un meme flux de donnees

    do{
        if(token->text[0] == '\n') break; //Aucune action si retour a la ligne

        struct node_s *word = new_node(NODE_VAR);
        if(!word) return NULL; //Verifie la presence d'une expression

        set_node_val_str(word, token->text);
        add_child_node(cmd, word);

    } while((token = tokenize(src)) != &eof_token);
    return cmd;
}