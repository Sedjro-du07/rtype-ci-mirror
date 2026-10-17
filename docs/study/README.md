# Comparative study

Why the engine is built the way it is: each section compares the options we considered, with
measurements where a number can settle the question. The decisions themselves are in `docs/adr/`;
this folder holds the evidence.

| Section | Question | Decision |
|---|---|---|
| [01 · ECS storage](01-ecs-storage.md) | How are components stored? | Paged sparse sets (ADR 0013) |

Other sections (networking, rendering, scripting, accessibility) are added as their tasks land.
