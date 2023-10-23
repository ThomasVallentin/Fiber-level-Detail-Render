# Real-time Cloth Rendering with Fiber-level Detail

Implémentation du papier [Real-time Cloth Rendering with Fiber-level Detail](https://people.csail.mit.edu/kuiwu/rtfr.html) portant sur le rendu de fibres de textiles en temps réel. Ce projet est réalisé dans le cadre du Master 2 Sciences de l'Image de l'Université Gustave Eiffel. 

[Lien vers la vidéo de démo](https://drive.google.com/file/d/1fXRfm8VVYQiQlAtiaFIic_FrY5VEQV0s/view?usp=sharing)

# Dépendances :

- [GLFW3 (3.3 stable)](https://github.com/glfw/glfw/tree/3.3-stable)
- [ImGui (docking branch)](https://github.com/ocornut/imgui/tree/docking)
- GLAD
- glimac
- [glm](https://github.com/g-truc/glm)
- [stb](https://github.com/nothings/stb)

# Installation

```
git clone --recurse-submodules https://github.com/ThomasVallentin/Fiber-level-Detail-Render.git
cd Fiber-level-Detail-Render
mkdir build
cd build
cmake ..
make
```
