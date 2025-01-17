# Rapport SAE 3.03 - Système mbash

## Contexte

Cette SAE consiste à créer un interpréteur de commandes simplifié en C, appelé `mbash`, </br> 
qui permet d'exécuter des commandes système tout en gérant l'historique des commandes, </br>
les opérations de base du shell (comme `cd`, `exit`, `history`) et quelques autres fonctionnalités.

## Objectif

L'objectif de ce projet est de développer un interpréteur de commandes en C qui simule un shell.
</br>Ce programme doit pouvoir exécuter des commandes simples et gérer des fonctionnalités de base </br>
comme la gestion de l'historique, et le déplacement dans l'arborescence de fichiers.

## Développement du projet

### 1. Structure du programme

Le programme est structuré autour de plusieurs modules permettant de gérer les commandes, </br>
les tokens, les variables d'environnement, et l'exécution de processus.

Les fonctionnalités principales incluent :

- **Parsing des commandes** : Chaque commande est analysée et découpée en tokens afin d'être interprétée et exécutée.

- **Gestion de l'historique** : Un système d'historique permet de naviguer dans les commandes </br>
précédemment saisies à l'aide des touches fléchées haut et bas.

- **Exécution des commandes** : Le programme peut exécuter des commandes systèmes en utilisant `execv` </br>
et gérer le fork d'un processus pour chaque commande.

### 2. Fonctionnalités principales

#### 2.1 Exécution des commandes

L'exécution des commandes se fait grâce à la fonction `do_exec_cmd`, qui utilise la fonction `execv` </br>
pour lancer un programme en tant que processus enfant.
</br>Le chemin du programme est recherché à travers les répertoires définis dans la variable d'environnement `PATH`.

#### 2.2 Gestion de l'historique

L'historique des commandes permet à l'utilisateur de revenir sur ses précédentes entrées en utilisant les touches fléchées haut et bas.
</br>Le programme conserve les dernières 100 commandes dans un tableau circulaire. </br>
Lors de l'exécution de la commande `history`, l'ensemble des commandes passées est affiché à l'écran.

#### 2.3 Autocomplétion

Une fonctionnalité d'autocomplétion permet de proposer des noms de fichiers et de répertoires présents </br>
dans le répertoire courant lorsque l'utilisateur appuie sur la touche `Tab`.
</br>Cela aide l'utilisateur à compléter les chemins de fichiers sans avoir à les taper entièrement.

#### 2.4 Commande `cd`

Le programme permet de changer de répertoire à l'aide de la commande `cd`.
</br>Cette commande modifie le répertoire de travail actuel du processus.

#### 2.5 Commande `exit`

La commande `exit` permet de quitter l'interpréteur de commandes.
</br>Elle met fin au programme.

## Conclusion

Le projet `mbash` permet de simuler un interpréteur de commandes basique avec une gestion de l'historique.
</br>Il sert de base pour comprendre la gestion des processus sous Unix et les fonctionnalités d'un shell. </br>
</br>La mise en œuvre a permis d'apprendre à manipuler les processus, les signaux et la gestion des entrées/sorties en C.

## Annexes

### Code source

Le code source du projet est disponible dans le fichier `mbash.c`.
</br></br>