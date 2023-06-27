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

2.Exécutez le fichier C++ avec la commande suivante (remplacez your-file.cpp par le nom du fichier sans le .cpp) :

                                              ./waf --run scratch/your-file

3.Pour changer les paramètres de la simulation à partir de la ligne de commande :

                                              ./waf --run "scratch/Leach --nodes=100 --cluster_percentage=0.05"

## Utilisation de NetAnim
1.Allez dans le répertoire netanim.

2.Exécutez NetAnim avec la commande suivante :

                                              ./Netanim
3.Dans l'interface de NetAnim, cliquez sur File puis Open.

4.Naviguez jusqu'au répertoire où votre fichier XML est généré (généralement le répertoire ns-3-dev-git) et sélectionnez le fichier XML.

5.Cliquez sur Play pour visualiser la simulation.

# Lancement du projet avec les fichiers du dossier Blockchain
On souhaite récupérer les informations des clusters (clusterhead et noeuds) et les stocker sur la blockchain ethereum (on utilise ici le testnet Fantom). Pour ce faire, on a implémenté watcher.js et watcher2.js qui surveillent les fichiers sur lesquels le code C++ de LEACH écrit ces données. watcher et watcher2 utilisent web3.js pour envoyer des transactions en utilisant les fonctions des smart contracts nodesStorage.sol et package.sol déployés sur ethereum.
Pour exécuter le projet, vous devez suivre les étapes suivantes:
1. Téléchargez le fichier blockchain
2. Installer la version 18.16 de node js (ou plus récénte)
3. Dans votre terminal, placez vous dans le répertoire blockchain et exécutez la commande suivante :
   
                                        npm install
4. Modifiez dans watcher et watcher2 les chemins d'accès (watcher vers Clusters.txt et watcher2 vers Packages.txt)
5. Lancez watcher et watcher 2 sur deux terminaux différents avec les commandes :

                             node watcher.js
                             node watcher2.js
6. Exécutez LEACH sur un troisième terminal comme indiqué dans la section précédente
