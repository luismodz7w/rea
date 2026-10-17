# REA: Token-Optimized Development

**Project**: Reverse Engineer Anything (rea-agents) — MCP for binary/app analysis  
**Size**: 71MB (15MB .git), ~6k lines code  
**Stack**: TypeScript, Node.js (22.19+), turborepo, MCP server

## Context-Saving Conventions

### File Read Strategy
- **Code entry points**: `src/index.ts`, `src/main.ts` — start here, not README
- **Lean exploration**: use Grep with `--head-limit` for first 20 matches before deep reads
- **Config files**: read once, cite by path instead of re-reading
- **Large outputs**: ask for `expand` if archived, never request full logs

### Safe Assumptions (Don't Ask)
- `package.json` structure: monorepo, MCP setup, standard npm layout
- TypeScript compiled to `dist/`, safe to skip source maps
- Test fixtures in `tests/fixtures/` are reference data, not examples
- Vendored code in `licenses/` is upstream, changes there break nothing

### Compression Rules
- Skip `node_modules/` entirely in all searches
- Exclude `*.lock`, `CHANGELOG.md`, old migration files
- Use `knip.json` to find dead code, don't read every file
- When tools say "bloated," grep for unused imports first

### Quality Signals
- Red flag: context > 60% — suggest `token-optimizer` audit
- Gold standard: one file read per question, cited by line number
- Healthy: each search returns <15 matches, if not, narrow the pattern

## Working in This Session

**Before starting work:**
1. Run `/token-optimizer` once (auto-audit, sets up hooks)
2. Set `TOKEN_OPTIMIZER_RUNTIME=claude` in your shell
3. Read this file if context starts filling up

**During coding:**
- Batch multiple edits into one write
- Use Edit tool, never sed/awk for large refactors
- Grep before Read for file location
- Ask for help narrowing searches instead of reading everything

**Session health:**
- If session hits 75% context: checkpoint or close and resume
- Dashboard link: `/token-optimizer` → "dashboard" option
- Cold resume: run `resume-lean` to reload checkpoint

## Project Topology

```
rea/
├── src/              # MCP server + CLI
├── dist/             # Compiled JS (safe to skip)
├── tests/            # Unit tests + fixtures
├── scripts/          # Build, setup, publish
├── docs/             # Docs site
├── packages/         # Workspace packages (if monorepo)
├── package.json      # Entry point: read first
├── tsconfig.json     # Build config (rarely changes)
└── turbo.json        # Turborepo pipeline
```

## Common Tasks (Token Budget)

| Task | Tokens | How to Save |
|------|--------|-------------|
| Understand MCP interface | 500 | Read `src/index.ts` + grep MCP exports |
| Add new tool | 2000 | Copy existing tool pattern + search `interface Tool` |
| Debug test failure | 1500 | Run test first, then grep error message in codebase |
| Review PR | 3000 | Search changed functions, read context only |
| Refactor module | 2500 | Grep `export` declarations, read callers |

## Token Optimizer Active

This project auto-runs:
- **Bash compression**: command output trimmed after each run
- **Read cache**: repeated file reads served from cache
- **Quality scoring**: context health updates live
- **Smart checkpoints**: before fanout, on failure, on session end

Hooks fire silently. If you see `[Token Optimizer]` messages, it's working.

---

**Last Updated**: 2026-10-17  
**Targets**: Claude Haiku 4.5+, all Sonnet/Opus tiers
