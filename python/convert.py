#!/usr/bin/env python3
"""
Convert YOLO PyTorch model to ONNX format
"""

import argparse
from pathlib import Path
from ultralytics import YOLO


def parse_args():
    """Parse command-line arguments"""
    parser = argparse.ArgumentParser(description="Convert YOLO model to ONNX format")
    parser.add_argument(
        "--input",
        type=str,
        default="models/yolov8s.pt",
        help="Path to input PyTorch model file (default: models/yolov8s.pt)",
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="Path to output ONNX model file (default: same as input with .onnx extension)",
    )
    parser.add_argument(
        "--opset", type=int, default=12, help="ONNX opset version (default: 12)"
    )
    parser.add_argument(
        "--dynamic",
        action="store_true",
        default=True,
        help="Enable dynamic batch/shape support (default: True)",
    )
    parser.add_argument(
        "--no-dynamic",
        dest="dynamic",
        action="store_false",
        help="Disable dynamic batch/shape support",
    )
    parser.add_argument(
        "--simplify",
        action="store_true",
        default=True,
        help="Run onnx-simplifier (default: True)",
    )
    parser.add_argument(
        "--no-simplify",
        dest="simplify",
        action="store_false",
        help="Skip onnx-simplifier",
    )
    return parser.parse_args()


def main():
    """Main conversion function"""
    args = parse_args()

    # Validate input file exists
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"❌ Error: Input file not found: {args.input}")
        return 1

    # Determine output path
    if args.output:
        output_path = Path(args.output)
    else:
        output_path = input_path.with_suffix(".onnx")

    print(f"🔄 Converting YOLO model to ONNX format")
    print(f"   Input:  {input_path}")
    print(f"   Output: {output_path}")
    print(f"   Opset:  {args.opset}")
    print(f"   Dynamic: {args.dynamic}")
    print(f"   Simplify: {args.simplify}")
    print()

    # Load model
    print("📦 Loading model...")
    model = YOLO(str(input_path))

    # Export to ONNX
    print("⚙️  Exporting to ONNX...")
    model.export(
        format="onnx",
        opset=args.opset,
        dynamic=args.dynamic,
        simplify=args.simplify,
    )

    print(f"✅ Successfully converted to ONNX: {output_path}")
    return 0


if __name__ == "__main__":
    exit(main())
