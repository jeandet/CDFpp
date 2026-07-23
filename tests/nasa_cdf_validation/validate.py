#!/usr/bin/env python3
"""Resaves a curated set of real CDF fixtures through CDFpp, then runs the NASA
CDF reference library's cdfvalidate tool against the output. This is the same
set of fixtures tests/simple_save's round-trip SCENARIO uses -- catches any
regression where CDFpp's own writer produces bytes the reference implementation
doesn't consider structurally valid, which a load()+compare round-trip through
CDFpp's own reader cannot catch (both sides would agree with each other while
still disagreeing with the spec).

Requires cdfvalidate to be built (not necessarily installed) somewhere: pass its
path via CDFVALIDATE, and make sure its runtime library is findable (typically
via LD_LIBRARY_PATH pointing at the NASA CDF distribution's lib/ or src/lib/).
"""
import os
import subprocess
import sys
import tempfile

import pycdfpp

FIXTURES = [
    "a_cdf.cdf",
    "a_compressed_cdf.cdf",
    "a_rle_compressed_cdf.cdf",
    "a_cdf_with_compressed_vars.cdf",
]


def main():
    data_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(__file__), "..", "resources")
    cdfvalidate = os.environ.get("CDFVALIDATE", "cdfvalidate")

    failures = []
    with tempfile.TemporaryDirectory() as out_dir:
        for name in FIXTURES:
            src = os.path.join(data_path, name)
            cdf = pycdfpp.load(src)
            if cdf is None:
                failures.append(f"{name}: CDFpp failed to load the fixture")
                continue
            out_path = os.path.join(out_dir, f"resaved_{name}")
            if not pycdfpp.save(cdf, out_path):
                failures.append(f"{name}: CDFpp failed to save")
                continue
            result = subprocess.run(
                [cdfvalidate, out_path], capture_output=True, text=True)
            print(f"--- {name} ---")
            print(result.stdout.strip())
            if result.returncode != 0:
                failures.append(
                    f"{name}: cdfvalidate exit {result.returncode}\n{result.stderr}")

    if failures:
        print("\nFAILURES:")
        for f in failures:
            print(" -", f)
        sys.exit(1)
    print(f"\nAll {len(FIXTURES)} CDFpp-saved fixtures passed NASA cdfvalidate.")


if __name__ == "__main__":
    main()
