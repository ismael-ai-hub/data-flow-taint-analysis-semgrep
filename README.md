# Data Flow and Taint Analysis in C with Semgrep

[![Semgrep Security Lab CI](https://github.com/ismael-ai-hub/data-flow-taint-analysis-semgrep/actions/workflows/semgrep-ci.yml/badge.svg)](https://github.com/ismael-ai-hub/data-flow-taint-analysis-semgrep/actions/workflows/semgrep-ci.yml)

An educational security lab investigating how untrusted input can reach sensitive SQL operations. This repository combines **manual MAY-taint analysis**, **custom Semgrep rules**, **positive and negative test cases**, and an investigation of **missing findings**.

> **Scope:** The C programs are intentionally simplified analysis fixtures. `read_user_input`, `execute_sql`, and transformation functions are declared but not implemented. The tests demonstrate *static-analysis behavior*, not exploitation of a running database or proof of SQL injection.

## Highlights

- Modeled the flow `read_user_input() → input → snprintf() → query → execute_sql()` using abstract states `IN` and `OUT`.
- Verified direct taint tracking, assignment propagation, conditional MAY flow, and rule-defined sanitizer behavior.
- Investigated two unreported flows in the local OSS scan: buffer-writing through `snprintf` and data crossing a function boundary.
- Demonstrated how treating a transformation without a verified security guarantee as a sanitizer **suppresses an alert without changing the C program**.

## Stack

C · Semgrep **1.186.0** (local **OSS** engine, as recorded in JSON reports) · Python **3.12** · Ubuntu/Docker · YAML · SAST

## Repository layout

```text
.
├── .github/
│   └── workflows/
│       └── semgrep-ci.yml      # CI rule validation and regression tests
├── programmes/                  # Seven C test fixtures
│   ├── vulnerable.c
│   ├── test-direct.c
│   ├── test-affectation.c
│   ├── test-sanitizer.c
│   ├── test-remove-prefix.c
│   ├── test-may.c
│   └── test-interprocedural.c
├── regles/
│   ├── taint-rule.yaml          # Baseline source, sink, and snprintf propagator
│   ├── taint-sanitizer.yaml     # Declares escape_sql a sanitizer
│   └── taint-faux-sanitizer.yaml # Intentionally trusts remove_prefix
├── resultats/                   # Nine exported JSON scan reports
├── docs/
│   ├── experiments.md           # Test matrix and evidence mapping
│   └── limitations.md           # Interpretation and security caveats
├── scripts/
│   └── check_findings.py       # Six automated expected-finding tests
├── README.md
└── .gitignore
```

## Data-flow model

The manual model tracks potentially tainted variables in `V = {input, query}`. The abstract domain is the powerset `P(V)`; control-flow joins use set union.

```text
read_user_input() → input → snprintf(...) → query → execute_sql(query)
```

| Node | Instruction | IN | OUT |
|---|---|---|---|
| n1 | `input = read_user_input()` | `∅` | `{input}` |
| n2 | `snprintf(query, ..., input)` | `{input}` | `{input, query}` |
| n3 | `execute_sql(query)` | `{input, query}` | `{input, query}` |

These are **manual abstract states for the lab's specified propagation model**, not internal Semgrep traces. `snprintf` limits output length but is not a SQL sanitizer; in production, use properly parameterized SQL queries.

## Verified experiments

The counts below were checked against the JSON files in `resultats/` (Semgrep 1.186.0, OSS engine). A count of zero is **not** a security guarantee.

| Test / configuration | Findings | Evidence |
|---|---:|---|
| Direct source → sink (`test-direct.c`, baseline rule) | 1 | `resultats/test-direct.json` |
| Assignment propagation (`test-affectation.c`, baseline) | 1 | `resultats/test-affectation.json` |
| Buffer output via `snprintf` (`vulnerable.c`, baseline) | 0 | `resultats/vulnerable.json` |
| `escape_sql` not trusted (`test-sanitizer.c`, baseline) | 1 | `resultats/test-sanitizer-sans-neutralisation.json` |
| `escape_sql` trusted (`test-sanitizer.c`, sanitizer rule) | 0 | `resultats/test-sanitizer.json` |
| `remove_prefix` not trusted (`test-remove-prefix.c`, baseline) | 1 | `resultats/remove-prefix-sans-sanitizer.json` |
| `remove_prefix` **incorrectly** trusted (`test-remove-prefix.c`, false-sanitizer rule) | 0 | `resultats/remove-prefix-avec-faux-sanitizer.json` |
| Branch merge / MAY flow (`test-may.c`, baseline) | 1 | `resultats/test-may.json` |
| Cross-function flow (`test-interprocedural.c`, baseline) | 0 | `resultats/test-interprocedural.json` |

The `snprintf($DST, ..., ..., $SRC)` pattern was independently confirmed to **match the call syntactically**, but the complete taint flow was not reported. The archives do **not** identify the exact reason that the custom propagation failed. A local OSS engine was confirmed from the `engine_requested` metadata; further engine/model experiments would be required to attribute the cause conclusively.

See [experiment details](docs/experiments.md) and [interpretation limits](docs/limitations.md).

## Reproduce locally

Requires Python 3.12 and Semgrep 1.186.0 (the versions tested for this project). From the repository root, using a Linux environment or compatible container:

```bash
python3 -m venv .venv-semgrep
source .venv-semgrep/bin/activate
python -m pip install 'semgrep==1.186.0'
semgrep --version
```

Validate and run **one rule configuration at a time**:

```bash
semgrep --validate --config regles/taint-rule.yaml
semgrep scan --config regles/taint-rule.yaml programmes/test-direct.c
semgrep scan --config regles/taint-rule.yaml programmes/vulnerable.c
semgrep scan --config regles/taint-rule.yaml programmes/test-may.c
semgrep scan --config regles/taint-rule.yaml programmes/test-interprocedural.c
semgrep scan --config regles/taint-rule.yaml programmes/test-remove-prefix.c
semgrep scan --config regles/taint-faux-sanitizer.yaml programmes/test-remove-prefix.c
semgrep scan --config regles/taint-rule.yaml programmes/test-sanitizer.c
semgrep scan --config regles/taint-sanitizer.yaml programmes/test-sanitizer.c
```

Run the six regression checks locally:

```bash
python scripts/check_findings.py
```

For JSON output:

```bash
semgrep scan --config regles/taint-rule.yaml --json programmes/test-direct.c
```

**Note:** All three YAML files deliberately reuse the same rule ID because they are alternative configurations of one experiment. Do not load all three together as a single ruleset; select the relevant YAML per test. The original Docker image used during the lab is not supplied in this repository, and a dedicated Dockerfile is future work.

## Continuous Integration — GitHub Actions

This project uses GitHub Actions to automatically validate custom Semgrep rules and execute security regression tests.

The CI pipeline performs the following checks:

- Installs Python 3.12 and Semgrep 1.186.0.
- Validates all three custom Semgrep rule configurations.
- Runs six automated taint-analysis regression tests.
- Compares observed findings against expected results.
- Fails the workflow if any regression test produces an unexpected result.

The test suite includes direct taint flows, assignment propagation, conditional MAY analysis, and sanitizer modeling.

**Why this matters:** The pipeline checks the consistency of security detection rules across code changes. Passing tests demonstrate expected analyzer behavior; they do not guarantee that the analyzed programs are free of vulnerabilities.

Workflow: [`.github/workflows/semgrep-ci.yml`](.github/workflows/semgrep-ci.yml)
Regression test script: [`scripts/check_findings.py`](scripts/check_findings.py)

The six assertions cover the known positive and sanitizer-modeling cases. The `snprintf` and interprocedural zero-finding observations are documented separately and are **not** treated as security passes.

## Lessons learned

1. A valid YAML rule and successful scan do not imply adequate detection coverage.
2. The absence of a finding is not proof that untrusted data cannot reach a sink.
3. A sanitizer is a **modeling assertion**; its security must be validated against real code and context.
4. MAY-taint analysis tracks *potential* contamination across control-flow alternatives.
5. Cross-function flows and buffer side effects need sufficient engine and library modeling.

## Next improvements

- Add regression fixtures for the currently unreported `snprintf` and cross-function flows, with clearly stated engine expectations.
- Provide a dedicated Dockerfile for reproducible local setup.
- Explore supported buffer-writing propagator models and cross-function analysis.
- Add defensive examples using parameterized SQL APIs.

## Disclaimer

Defensive educational project. The rules and examples do not constitute a production-grade SQL injection detector, and the missing function implementations mean no exploitability claim is established.
