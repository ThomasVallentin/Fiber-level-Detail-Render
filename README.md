# Fiber-level-Detail-Render

## Biblio

Le papier d'origine : https://people.csail.mit.edu/kuiwu/rtfr.html

### Pour aller plus loin ?

#### Lighting / Shading

- Infos sur la génération et le rendu des cheveux dans le moteur Frostbite (piste pour améliorer le rendu des fibres ?) : http://advances.realtimerendering.com/s2019/hair_presentation_final.pdf

#### Simulation (nouvel aspect qu'on pourrait ajouter au projet ?)

- Simulation de tissu à base de fils de laine : http://visualcomputing.ist.ac.at/publications/2020/HYLC
    - Très adapté à notre cas mais simulation très loin d'etre temps réelle.

- Simulation temps réelle de vetements basée sur le GPU : https://www.researchgate.net/publication/338392160_GPU-based_Real-time_Cloth_Simulation_for_Virtual_Try-on
    - C'est vraiment temps réel et une implémentation est dispo sur github : https://github.com/sutongkui/simulator. Cependant comme la simulation traite des maillages et non des "yarns", il faudra également trouver un moyen de déformer nos courbes pour qu'elles suivent la simulation, en les wrappant dessus par exemple)
