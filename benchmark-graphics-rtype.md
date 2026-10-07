# Benchmark des bibliothèques graphiques C++ pour le moteur R-Type

## 1. Objectif

Choisir la bibliothèque graphique (rendu, fenêtre, audio, input) sur laquelle construire le moteur du R-Type, en respectant les contraintes du sujet Epitech B-CPP-500.

## 2. Contraintes du sujet prises en compte

| Contrainte | Impact sur le choix |
|---|---|
| Langage C++ | Une API C++ native s'intègre plus naturellement, une API C doit être encapsulée |
| Linux obligatoire, Windows attendu (cross-platform) | Les API propres à un OS sont éliminées |
| Package manager obligatoire (Conan, vcpkg, CPM) | La bibliothèque doit être facile à récupérer sans installation système |
| Moteurs existants et bibliothèques au périmètre trop large interdits | Unreal, Godot, Unity et équivalents sont éliminés |
| Sous-systèmes Rendering / Networking / Game Logic découplés | La bibliothèque doit pouvoir être cachée derrière une interface |
| Partie 2 : moteur standalone, modulaire, avec outils (console, métriques, éditeur) | Bonus pour une bonne intégration avec Dear ImGui |
| Jeu 2D (shoot'em up horizontal) avec son | Rendu 2D efficace et audio intégré privilégiés |
| Délai de 6 semaines | Les solutions très bas niveau coûtent trop de temps |

Légende des verdicts : ✅ retenu, ⚠️ possible mais pas idéal, ❌ éliminé, 🧩 complément utile.

## 3. Inventaire complet par catégorie

### 3.1 Fenêtrage + 2D multimédia

| Bibliothèque | Langage | Verdict | Raison |
|---|---|---|---|
| SFML 3 | C++17 | ✅ | API C++ moderne, rendu 2D + audio + input en une dépendance, recommandée par le sujet |
| SFML 2.6 | C++03/11 | ⚠️ | Stable mais en maintenance, audio via OpenAL, version à forcer dans vcpkg (voir section 5) |
| SDL3 | C | ✅ | Standard de l'industrie, meilleur support manette, explicitement autorisée |
| SDL2 | C | ⚠️ | Toujours solide mais remplacée par SDL3, pas de raison de partir dessus sur un projet neuf |
| Raylib | C | ⚠️ | Très simple et complète, explicitement autorisée, mais état global difficile à abstraire proprement |
| Allegro 5 | C | ⚠️ | Viable pour la 2D, mais communauté réduite et intégration outillage faible |
| CSFML | C | ❌ | Binding C de SFML : aucun intérêt en C++ alors que SFML est native |

### 3.2 Fenêtrage + contexte pour API 3D

| Bibliothèque | Langage | Verdict | Raison |
|---|---|---|---|
| GLFW | C | ⚠️ | Excellente base, mais ne fournit que fenêtre + input : tout le rendu 2D et l'audio sont à faire |
| GLUT / freeglut | C | ❌ | Datée, remplacée par GLFW |

### 3.3 API 3D bas niveau et couches d'abstraction

| Bibliothèque | Plateformes | Verdict | Raison |
|---|---|---|---|
| OpenGL (+ GLAD / GLEW) | Toutes | ⚠️ | Formateur, mais il faut recoder sprite batching, textures et shaders : trop coûteux pour la partie 1 |
| Vulkan | Toutes | ❌ | Extrêmement verbeux, disproportionné pour un shmup 2D en 6 semaines |
| DirectX 11/12 | Windows | ❌ | Pas cross-platform |
| Metal | Apple | ❌ | Pas cross-platform, et macOS ne compte pas comme cross-platform dans le sujet |
| bgfx | Toutes | ⚠️ | Abstraction de rendu puissante, mais complexe et orientée 3D, risque de périmètre jugé trop large |
| Diligent Engine | Toutes | ❌ | Très complet, trop lourd et proche d'un moteur |
| wgpu / Dawn (WebGPU) | Toutes | ❌ | Prometteur mais encore jeune, intégration et packaging plus délicats |

### 3.4 Moteurs et frameworks 3D

| Bibliothèque | Verdict | Raison |
|---|---|---|
| Unreal Engine | ❌ | Moteur existant, interdit par le sujet |
| Godot | ❌ | Moteur existant, interdit par le sujet |
| O3DE | ❌ | Moteur existant, interdit |
| Ogre3D | ❌ | Moteur de rendu complet, périmètre trop large |
| Irrlicht | ❌ | Moteur 3D complet, périmètre trop large et vieillissant |
| Magnum | ❌ | Framework de moteur, ferait une grosse partie du travail demandé |
| Urho3D | ❌ | Moteur complet, interdit |
| Filament | ❌ | Moteur de rendu PBR 3D, hors sujet pour de la 2D |
| The Forge | ❌ | Framework de rendu complet, trop large et trop complexe |

### 3.5 Interfaces graphiques (GUI)

| Bibliothèque | Verdict | Raison |
|---|---|---|
| Dear ImGui | 🧩 | Pas une base de rendu, mais idéale pour les outils du moteur : console dev, métriques, éditeur de niveaux |
| Qt | ⚠️ | Possible uniquement pour un éditeur séparé, très lourd à packager et non adapté à une boucle de jeu |
| wxWidgets | ❌ | Widgets natifs pour applis de bureau, pas pour un jeu |
| GTK / gtkmm | ❌ | Orientée Linux, pas adaptée à un jeu |
| FLTK | ❌ | Applis de bureau légères, hors sujet |
| Nana | ❌ | Applis de bureau, communauté réduite |
| Ultimate++ | ❌ | Framework d'appli complet, hors sujet |
| JUCE | ❌ | Orientée audio et plugins, hors sujet |
| Slint | ❌ | Interfaces déclaratives d'applis, pas de rendu jeu |
| Elements | ❌ | Bibliothèque GUI de niche, hors sujet |
| WinAPI / MFC | ❌ | Windows uniquement |

### 3.6 Dessin 2D vectoriel

| Bibliothèque | Verdict | Raison |
|---|---|---|
| Cairo | ❌ | Rendu vectoriel orienté documents, trop lent pour un jeu temps réel |
| Skia | ❌ | Très puissante mais énorme à compiler et à packager |
| Blend2D | ❌ | Rendu vectoriel CPU, pas adapté aux sprites |
| NanoVG | ⚠️ | Utile pour une UI vectorielle par-dessus OpenGL, pas comme base |
| AGG | ❌ | Ancienne, rendu CPU |

### 3.7 Tracé de graphiques et data viz

| Bibliothèque | Verdict | Raison |
|---|---|---|
| ImPlot | 🧩 | Intéressant pour les métriques réseau (lagometer, bande passante) dans les outils ImGui |
| matplotlib-cpp | ❌ | Dépend de Python, hors sujet |
| QCustomPlot / Qt Charts | ❌ | Dépend de Qt |
| gnuplot-iostream | ❌ | Dépend d'un programme externe |
| VTK | ❌ | Visualisation scientifique, énorme |
| sciplot | ❌ | Tracé statique, hors sujet |

### 3.8 Utilitaires souvent associés

| Bibliothèque | Verdict | Raison |
|---|---|---|
| GLM | ⚠️ | Utile seulement si rendu OpenGL maison, SFML et SDL fournissent déjà leurs vecteurs |
| stb_image | ⚠️ | Utile seulement avec GLFW + OpenGL, déjà couvert par SFML et SDL_image |
| Assimp | ❌ | Import de modèles 3D, inutile en 2D |
| FreeType | ⚠️ | Déjà intégrée dans SFML, à ajouter seulement avec un rendu maison |

## 4. Comparatif des finalistes

Notes sur 5, total sur 50.

| Critère | SFML 3 | SDL3 | SDL2 | SFML 2.6 | Raylib | GLFW + OpenGL | Allegro 5 |
|---|---|---|---|---|---|---|---|
| Facilité de prise en main | 5 | 3 | 3 | 5 | 5 | 1 | 3 |
| Rendu 2D (sprites, scrolling) | 5 | 4 | 4 | 5 | 5 | 2 | 4 |
| Audio intégré | 5 | 3 | 3 | 4 | 5 | 0 | 4 |
| Input et manettes | 3 | 5 | 5 | 3 | 4 | 3 | 3 |
| Disponibilité vcpkg / Conan / CPM | 5 | 5 | 5 | 3 | 4 | 5 | 3 |
| Cross-platform Linux / Windows | 5 | 5 | 5 | 5 | 5 | 5 | 4 |
| Facilité d'encapsulation derrière une interface | 5 | 4 | 4 | 4 | 3 | 4 | 3 |
| Intégration Dear ImGui | 5 | 5 | 5 | 5 | 3 | 5 | 2 |
| Communauté et documentation | 4 | 5 | 5 | 5 | 4 | 5 | 2 |
| Valeur perçue en soutenance | 4 | 5 | 4 | 3 | 3 | 5 | 2 |
| **Total / 50** | **46** | **44** | **43** | **42** | **41** | **35** | **30** |

### Analyse rapide

**SFML 3** offre le meilleur équilibre : API C++ native, tout le multimédia dans une seule dépendance, packaging simple et choix explicitement recommandé par le sujet. Son point faible est le support manette, moins riche que celui de SDL.

**SDL3** est l'alternative la plus sérieuse. Son support manette et son remapping sont un vrai argument pour l'accessibilité motrice, mais son API C demande plus de code d'encapsulation, et l'audio et les images passent par des bibliothèques annexes.

**Raylib** est excellente pour prototyper, mais son état global rend une abstraction propre plus difficile, et elle peut être perçue comme un raccourci.

**GLFW + OpenGL** est la solution la plus formatrice, mais elle oblige à recoder une grande partie de ce que SFML fournit déjà, ce qui met en danger la livraison de la partie 1.

## 5. Focus : SFML 2.6 vs SFML 3

| Critère | SFML 2.6 | SFML 3 |
|---|---|---|
| Standard C++ | C++03/11 | C++17 |
| État du projet | Maintenance uniquement, dernière branche 2.x | Développement actif |
| Événements | Union + enum, gros switch | Événements typés, plus sûrs |
| Enums | Non scopés | Scopés |
| Gestion des erreurs | Booléens de retour | `std::optional` et constructeurs explicites |
| Audio | OpenAL (dépendance supplémentaire) | miniaudio (embarqué) |
| Packaging Windows | Plus délicat à cause d'OpenAL | Plus simple |
| vcpkg | Version à forcer, le port par défaut est la 3 | Version par défaut |
| Conan / CPM | Disponible | Disponible |
| ImGui-SFML | Branche 2.6 | Branche 3 |
| Tutoriels et réponses en ligne | Très nombreux | Moins nombreux, en hausse |
| Compatibilité avec CSFML vu en 1re année | Logique proche | API modernisée, quelques réflexes à changer |
| Argument pour l'étude comparative | Choix d'une version figée | Version moderne et maintenue |

**Le seul vrai avantage de SFML 2.6** est la masse de ressources disponibles. C'est aussi un piège avec SFML 3 : beaucoup d'exemples en ligne ne compilent plus tels quels.

**SFML 2.6 ne se justifie que si l'équipe possède déjà une base de code SFML 2 réutilisable.** Pour un projet neuf, SFML 3 est le choix logique.

## 6. Recommandation finale

**Bibliothèque retenue : SFML 3**, avec SDL3 comme alternative documentée.

Recommandations d'architecture :

- Placer la bibliothèque derrière des interfaces du moteur (rendu, audio, input), afin qu'aucun header SFML ne soit inclus en dehors du backend.
- Garder le serveur totalement indépendant de SFML : il ne contient que l'ECS, le réseau et la logique de jeu.
- En partie 2, livrer le backend graphique sous forme de plugin chargé au runtime pour démontrer la modularité. Un second backend SDL3 prouverait que le moteur est réellement générique.

## 7. Stack complémentaire recommandée

| Besoin | Bibliothèque | Rôle |
|---|---|---|
| Réseau | Asio | UDP asynchrone et serveur multithreadé, plus adapté que le module réseau de SFML |
| Outils du moteur | Dear ImGui (+ ImGui-SFML) | Console développeur, métriques, éditeur de niveaux |
| Métriques visuelles | ImPlot | Lagometer, FPS, bande passante |
| Scripting | Langage de script maison | Comportements des entités sans recompilation, conçu sur mesure pour le moteur (Lua via sol2 reste l'alternative de repli) |
| ECS | Implémentation maison | Une bibliothèque comme EnTT ferait le cœur du travail évalué |
