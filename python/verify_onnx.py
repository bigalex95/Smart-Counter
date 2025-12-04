#!/usr/bin/env python3
"""
Verify ONNX model validity
"""

import argparse
from pathlib import Path
import onnx


def parse_args():
    """Parse command-line arguments"""
    parser = argparse.ArgumentParser(description="Verify ONNX model validity")
    parser.add_argument(
        "model",
        type=str,
        nargs="?",
        default="models/yolov8s.onnx",
        help="Path to ONNX model file (default: models/yolov8s.onnx)",
    )
    parser.add_argument(
        "--verbose", action="store_true", help="Print detailed model information"
    )
    return parser.parse_args()


def main():
    """Main verification function"""
    args = parse_args()

    # Validate input file exists
    model_path = Path(args.model)
    if not model_path.exists():
        print(f"❌ Error: Model file not found: {args.model}")
        return 1

    print(f"🔍 Verifying ONNX model: {model_path}")
    print()

    try:
        # Load model
        print("📦 Loading model...")
        onnx_model = onnx.load(str(model_path))

        # Check model validity
        print("✔️  Checking model...")
        onnx.checker.check_model(onnx_model)

        print("✅ ONNX model is valid!")

        # Print additional info if verbose
        if args.verbose:
            print()
            print("📊 Model Information:")
            print(f"   IR Version: {onnx_model.ir_version}")
            print(
                f"   Producer: {onnx_model.producer_name} {onnx_model.producer_version}"
            )
            print(f"   Graph name: {onnx_model.graph.name}")
            print(f"   Number of nodes: {len(onnx_model.graph.node)}")

            if onnx_model.graph.input:
                print(f"\n   Inputs:")
                for inp in onnx_model.graph.input:
                    print(f"      - {inp.name}: {inp.type}")

            if onnx_model.graph.output:
                print(f"\n   Outputs:")
                for out in onnx_model.graph.output:
                    print(f"      - {out.name}: {out.type}")

        return 0

    except Exception as e:
        print(f"❌ Model verification failed: {e}")
        return 1


if __name__ == "__main__":
    exit(main())
