# 42cursus — milestones (ranks) y proyectos

Leído de la intra el 2026-09-24 y revisado el 2026-10-25 (cuenta `xalara`, campus 46 =
Barcelona), del mismo JSON que alimenta el holy graph:

```
https://projects.intra.42.fr/project_data.json?cursus_id=21&campus_id=46
```

## Cursus que existen en esta cuenta

| id | cursus | aquí |
|---|---|---|
| 9  | C Piscine | `packs/piscine/` (20 packs, hechos) |
| 66 | C-Piscine-Reloaded | un solo proyecto: `c-piscine-reloaded` (**in_progress**, ~49 h, solo) |
| 21 | 42cursus | el common core: 7 milestones, ranks 00–06 |

**C-piscine-reloaded es un cursus aparte**, no un proyecto del 42cursus: es el puente
entre la piscine y el common core.

## 42next: el camino de esta cuenta

Dentro del 42cursus conviven dos caminos: el antiguo y **42next**, que es una *quest*.
Esta cuenta va por 42next, y eso decide qué proyectos le tocan:

- los proyectos de 42next piden *haber validado* la quest `42next`
  (`42next-push_swap`, `42next-born2beroot`, `a-maze-ing`, los Python Module...);
- los del camino antiguo piden *no haberla validado*, así que aquí no se hacen:
  so_long, FdF, fract-ol, minitalk, pipex, minishell, Philosophers, cub3d, miniRT,
  CPP Modules, webserv, ft_irc;
- hay proyectos repetidos con dos slugs, uno por camino: `42cursus-push_swap` (v10.0,
  el clásico) frente a `42next-push_swap` (v1.0, cuatro estrategias y en pareja), y
  `born2beroot` frente a `42next-born2beroot`. **El enunciado bueno es siempre el del
  slug `42next-`**; el otro también se puede abrir en la intra y engaña.

## Milestones de 42next

El milestone es la *quest* `common-core-rank-NN`. Un proyecto pertenece al rank que sus
reglas dicen que aún no tienes validado ("You must not have validated any of quests
common-core-rank-NN"), y exige tener terminado el rank anterior.

| rank | proyectos | pack |
|---|---|---|
| 00 | Libft | `M0/` |
| 01 | ft_printf · get_next_line · push_swap · Exam atonement | `M1/` |
| 02 | Born2beroot · A-Maze-ing · Python Module 00–10 (y 05 bis, 06 bis) · Exam Rank 02 | `M2/` |
| 03 | Codexion · Call Me Maybe · Fly-in · Exam Rank 03 | `M3/` |
| 04 | NetPractice · Pac-Man · RAG against the machine · Exam Rank 04 | `M4/` |
| 05 | Inception · Agent Smith · The Answer Protocol · tree_nity · Exam Rank 05 | `M5/` |
| 06 | ft_transcendence · ft_resume · Exam Rank 06 | `M6/` |

Requisitos que no se ven en la tabla:

- Born2beroot, A-Maze-ing y Python Module 00 piden además la quest `42bcn-libft-exam`.
  Exam atonement (rank 01) es la vía para quien no la tiene.
- Python Module 05 bis y 06 bis son alternativas a 05 y 06 (Python Module 09 pide
  "1 of python-module-06-bis and python-module-08"). Sus enunciados están en los packs
  de 05 y 06 como `bis.en.subject.pdf` y `subject_bis.txt`.

Slugs de la intra (los que no son evidentes): `42cursus-libft`, `42cursus-ft_printf`,
`42cursus-get_next_line`, `42next-push_swap`, `42next-born2beroot`, `a-maze-ing`,
`python-module-NN`, `python-module-05-bis`, `python-module-06-bis`,
`42next-exam-rank-NN`, `42_collaborative_resume` (= ft_resume).

Los enunciados del rank 02 solo existen en inglés (`en.subject.pdf`); el de
`42next-push_swap` sí está en castellano.

El resto del 42cursus (195 proyectos en el JSON) son las ramas de después del common
core, agrupadas por *capas* temáticas — Algo & AI & Data, Security, Devops & Network,
Web & Mobile, System & Kernel, Graphics & Gaming, Cryptography & Maths, Development,
Professional Experience — no por rank.
