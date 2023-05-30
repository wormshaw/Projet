# NS-3 Installation et Utilisation Guide
Ce README vous guidera à travers l'installation de la dernière version de NS-3, l'exécution d'un fichier C++ et l'utilisation de NetAnim pour visualiser les résultats de notre simulation.

## Prérequis
Assurez-vous que votre système d'exploitation est à jour. Ce guide est écrit pour les utilisateurs de Linux.

## Installation de NS-3
1.Ouvrez un terminal.
2.Mettez à jour votre système avec la commande suivante :
                                              sudo apt-get update
                                              sudo apt-get upgrade
                                              
3.Installez les dépendances nécessaires avec la commande suivante :
                                              sudo apt-get install gcc g++ python python3 python3-dev qt5-default mercurial python-setuptools git cmake libc6-dev libc6-dev-i386 g++-multilib gdb valgrind gsl-bin libgsl-dev libgsl23 libgslcblas0 tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev libgtk2.0-0 libgtk2.0-dev uncrustify doxygen graphviz imagemagick texlive texlive-latex-extra texlive-generic-recommended texinfo dia texlive texlive-latex-extra texlive-extra-utils texlive-generic-recommended texi2html python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev python-pygccxml

4.Clonez le dépôt NS-3 avec la commande suivante :
                                              git clone https://github.com/nsnam/ns-3-dev-git.git
                                              
5.Ensuite suivez les instructions sur le site de NS3.

## Exécution d'un fichier C++
1.Placez le fichier C++ dans le répertoire scratch de NS-3.
2.Exécutez le fichier C++ avec la commande suivante (remplacez your-file.cpp par le nom du fichier) :
                                              ./waf --run scratch/your-file

## Utilisation de NetAnim
1.Allez dans le répertoire netanim
2.Exécutez NetAnim avec la commande suivante :
                                              ./Netanim
3.Dans l'interface de NetAnim, cliquez sur File puis Open.
4.Naviguez jusqu'au répertoire où votre fichier XML est généré (généralement le répertoire ns-3-dev-git) et sélectionnez le fichier XML.
5.Cliquez sur Play pour visualiser la simulation.

# Lancer l'API pour stocker les informations de LEACH sur la blockchain
1. Une fois le fichier avec les informations de LEACH récupéré, lancez votre invite de commandes et placez vous dans le répertoire project
2. exécutez la commande : node index.js
3. puis ouvrez postman, créez une commande post, et mettez l'URL http://localhost:3000/test
4. cliquez sur raw puis sélectionnez JSON pour le type de text
5. mettez dans la zone de texte :
{
  "filePath": "chemin\\d_acces\\vers\\votre\\fichier.txt"
}
6.Lancez la requête en appuyant sur Send
