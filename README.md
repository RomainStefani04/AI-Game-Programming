# AI-Game-Programming
Master informatique Université Côte d'Azur

# Authors
- Romain STEFANI
- Guillaume FAURE

# IA pour la compétition
IA pour la Compétition

**PVS (Principal Variation Search) :** Variante d'Alpha-Beta qui explore le premier coup normalement, puis teste les autres avec une fenêtre quasi-nulle [alpha, alpha+1] pour les éliminer rapidement. Si un coup dépasse, on le re-recherche avec la fenêtre complète.

**Killer Moves :** On mémorise les 2 derniers coups qui ont causé une coupure à chaque profondeur, et on les explore en priorité dans les positions similaires. Un coup qui a coupé ailleurs a de bonnes chances de couper ici aussi.

**Table de Transposition :** Cache de positions avec leur score, profondeur et meilleur coup. Quand on retombe sur une position déjà vue, on réutilise le résultat au lieu de tout recalculer.

**Iterative Deepening :** On recherche à profondeur 1, puis 2, puis 3... jusqu'au timeout (3s). Garantit un coup légal même si interrompu.

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