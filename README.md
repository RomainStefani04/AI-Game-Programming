# AI-Game-Programming
Master informatique Université Côte d'Azur

# Authors
- Romain STEFANI
- Guillaume FAURE

# IA pour la compétition
PVS 


(Les autres ia ont été utilisé pour tester des méthodes ou pour faire des compétitions internes mais sont moins efficaces)

# Utilisation

### Pour utiliser nos tests à nous : main, simulation, tournament

On peut lancer le main avec la commande suivante :

```bash
# Pour main
make main

# Pour simulation
make simulation

# Pour tournament
make tournament
```
Puis exécuter le binaire généré avec :

```bash
# Pour main
.\target\main

# Pour simulation
.\target\simulation

# Pour tournament
.\target\tournament
```

### Pour la compétition avec d'autres ia :

On a une classe c external_player qui prend une de nos ia et permet de l'adapter pour qu'elle ai un "langage" commun avec les autres ia de la compétition.

Pour générer l'executable utilisé dans la classe arbitre de la compétition qui va faire jouer les 2 joueurs, on peut utiliser la commande suivante :

```bash
make external
```

L'exécutable sera généré dans le dossier target sous le nom external_player.exe

Il restera ensuite qu'à modifier le chemin des executables dans la classe arbitre pour choisir les joueurs qui vont s'affronter.