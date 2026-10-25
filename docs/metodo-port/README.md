# Método de port (spec)

Cómo se descompila y se comprueba cada función en este proyecto, y cómo
mejorarlo para los próximos ports. Es Markdown normal para que lo use
cualquier persona o herramienta (Claude Code, Cursor, opencode, Kiro, Cline…).

| Archivo | Qué tiene |
| --- | --- |
| [requirements.md](requirements.md) | Los requisitos del método (R1–R9), con su estado: HECHO, PARCIAL o NUEVO |
| [design.md](design.md) | El flujo de un hito, las piezas (herramientas), los casos grabados con el emulador, la cobertura, la tabla de trampas y los emuladores de cada consola |
| [tasks.md](tasks.md) | Las tareas para implementar lo NUEVO, en orden de prioridad |
| [nuevo-proyecto.md](nuevo-proyecto.md) | Cómo empezar otro juego u otra consola con este método, paso a paso |

Las reglas de trabajo de cada hito (las que no se pueden romper) están en
[`AGENTS.md`](../../AGENTS.md). Este spec las explica y las amplía; si alguna
vez dicen cosas distintas, manda `AGENTS.md` hasta que se actualice.

## Cómo usarlo con cada herramienta

Todas entran por `AGENTS.md`, que enlaza aquí:

| Herramienta | Qué lee |
| --- | --- |
| Claude Code | `CLAUDE.md` (importa `AGENTS.md`) |
| Cursor | `AGENTS.md` y `.cursor/rules/metodo-port.mdc` |
| opencode, Codex y otros | `AGENTS.md` |
| Kiro | `.kiro/steering/flujo.md` y `.kiro/steering/metodo-port.md` |
| Cline | `.clinerules` |

Para implementar una tarea de `tasks.md`, pídele a la herramienta: «haz la
tarea N de `docs/metodo-port/tasks.md`». Cuando se termine, se marca con
`[x]`.
