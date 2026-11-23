# Security and analysis limitations

1. **No runtime SQL implementation.** Sources and sinks are declarations only. Scans identify modeled data flows and do not prove exploitability.
2. **MAY analysis and false positives.** Potential flows are not necessarily executable attacks; infeasible paths and unsupported contextual knowledge can affect alerts.
3. **Missing alerts and false negatives.** A clean scan is not proof of security. The `snprintf` and interprocedural fixtures had zero findings despite manually anticipated data flow.
4. **Sanitizer trust.** `escape_sql` is not implemented or independently verified. The intentionally incorrect `remove_prefix` sanitizer shows that a rule can suppress a potentially important finding.
5. **Engine and API modeling.** Reports identify `engine_requested: OSS`; propagation through side effects and across function boundaries depends on engine/model support. The root cause of the missing `snprintf` finding has not been conclusively isolated.
6. **Scope of rules.** The rule targets artificial functions (`read_user_input` and `execute_sql`), not arbitrary SQL interfaces. It must not be described as a production-grade general SQL injection detector.
7. **Reproducibility.** Local OS, dependency versions, and future Semgrep releases can change results. Versions are documented, but the original Docker image is not included.
8. **Safer design.** For real database systems, use parameterized queries and review the actual API and query-building context.

## Interpreting results

- **Finding = 1:** the engine recognized a source-to-sink flow under the chosen rule; this is not an exploit proof.
- **Finding = 0:** the engine did not report a matching flow under the chosen rule; this is not a safety proof.
- **False positive / false negative:** require a defined security ground truth; the simplified fixtures chiefly demonstrate model behavior and coverage limits.
