# Mbash: 
**Création d'un mini-Bash en C**

<u>Etape 1:</u> 
Initialisation du programme:
</br>- Création de la boucle interactive
</br>- Affichage d'un $ et possibilité d'écrire
</br>- Texte entré retourné, **exit** fonctionnel
</br>
</br>
</br>
</br>

#### Lecture de la commande:
- **fgets()** dans `main()`

#### Analyse de la commande:
- **parse_and_execute()**:
    - Ignore les espaces avec `sauter_espaces()`,
    - Appelle `tokenize()`,
    - Construit des commandes avec `parse_command()`.

#### Exécution de la commande:
- **do_simple_command()**:
    - Alloue dynamique la place avec `malloc()`,
    - Met en place les arguments avec `strcpy()`,
    - Crée un processus fils avec `fork()`.

#### Recherche et exécution de la commande:
- **do_exec_cmd()**:
    - Appelle `execv()` pour exécuter la commande,
    - Appelle `search_path()` si le chemin n'est pas absolu,
    - Gère des potentielles erreurs d'exécution avec `strerror()`,
    - Attend le processus fils avec `waitpid()`.

