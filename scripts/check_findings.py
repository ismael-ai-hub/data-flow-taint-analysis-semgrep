
import json
import subprocess
import sys

TESTS = [
    ("regles/taint-rule.yaml", "programmes/test-direct.c", 1),
    ("regles/taint-rule.yaml", "programmes/test-affectation.c", 1),
    ("regles/taint-rule.yaml", "programmes/test-may.c", 1),
    ("regles/taint-sanitizer.yaml", "programmes/test-sanitizer.c", 0),
    ("regles/taint-rule.yaml", "programmes/test-remove-prefix.c", 1),
    ("regles/taint-faux-sanitizer.yaml", "programmes/test-remove-prefix.c", 0),
]

failures = 0

for rule, target, expected in TESTS:
    result = subprocess.run(
        [
            "semgrep", "scan",
            "--config", rule,
            "--json",
            "--quiet",
            target,
        ],
        capture_output=True,
        text=True,
    )

    try:
        report = json.loads(result.stdout)
    except json.JSONDecodeError:
        print(f"ERROR: Invalid JSON for {target}")
        print(result.stderr)
        failures += 1
        continue

    errors = report.get("errors", [])
    if errors or result.returncode not in (0, 1):
        print(f"ERROR: Scan failed for {target}")
        print(errors or result.stderr)
        failures += 1
        continue

    found = len(report.get("results", []))
    status = "PASS" if found == expected else "FAIL"

    print(f"{status}: {target} | expected={expected}, found={found}")

    if found != expected:
        failures += 1

if failures:
    print(f"\n{failures} test(s) failed.")
    sys.exit(1)

print("\nAll Semgrep regression tests passed.")
