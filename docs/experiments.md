# Experiment log

## Environment and validation

- Python: 3.12.3
- Semgrep: 1.186.0
- Engine requested in saved JSON reports: `OSS`
- Seven C fixtures and three alternative YAML configurations
- Nine JSON reports, each with `errors: []`
- C fixtures passed `gcc -std=c11 -Wall -Wextra -Wpedantic -fsyntax-only` in the portfolio audit environment; no executable linking is expected because functions are intentionally unimplemented.

## Test matrix

| Fixture | Config | Findings | Report |
|---|---|---:|---|
| `test-direct.c` | `taint-rule.yaml` | 1 | `test-direct.json` |
| `test-affectation.c` | `taint-rule.yaml` | 1 | `test-affectation.json` |
| `vulnerable.c` | `taint-rule.yaml` | 0 | `vulnerable.json` |
| `test-sanitizer.c` | `taint-rule.yaml` | 1 | `test-sanitizer-sans-neutralisation.json` |
| `test-sanitizer.c` | `taint-sanitizer.yaml` | 0 | `test-sanitizer.json` |
| `test-remove-prefix.c` | `taint-rule.yaml` | 1 | `remove-prefix-sans-sanitizer.json` |
| `test-remove-prefix.c` | `taint-faux-sanitizer.yaml` | 0 | `remove-prefix-avec-faux-sanitizer.json` |
| `test-may.c` | `taint-rule.yaml` | 1 | `test-may.json` |
| `test-interprocedural.c` | `taint-rule.yaml` | 0 | `test-interprocedural.json` |

Reports are located in `../resultats/`.

## Focused investigations

### Direct and assignment-based flow

Both the nested `execute_sql(read_user_input())` call and the assignment chain `input → query → execute_sql` produced a finding. These serve as positive controls for the source and sink patterns.

### `snprintf` buffer write

The standalone syntax pattern `snprintf($DST, ..., ..., $SRC)` matched the C call once, but the taint scan reported zero findings. This is an observed coverage gap for the tested configuration, not proof of a safe SQL query and not proof of one specific engine defect.

### `escape_sql` sanitizer

With the same C fixture, the original rule produced one finding and the rule declaring `escape_sql(...)` as a sanitizer produced zero. The real transformation is not implemented, so the experiment tests *analysis assumptions*, not actual SQL escaping quality.

### `remove_prefix` false sanitizer

The original rule produced one finding and the artificially trusting rule produced zero. Label this a **potential false-negative mechanism**, not a demonstrated real-world exploit.

### MAY analysis

`test-may.c` has one untainted and one potentially tainted branch; the sink is reported once at their join.

### Interprocedural analysis

In `test-interprocedural.c`, data passes as an argument to a second function that calls the sink. The local scan reported zero findings. The saved reports confirm the OSS engine; this test does not establish capabilities of other Semgrep editions.
