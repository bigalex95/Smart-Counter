from typing import List


def measure_fps_from_seconds(times_s: List[float]) -> float:
    """Return average FPS from a list of per-frame times in seconds.

    Returns 0.0 if list is empty or average time is zero.
    """
    if not times_s:
        return 0.0
    avg_s = sum(times_s) / len(times_s)
    if avg_s == 0:
        return 0.0
    return 1.0 / avg_s


def measure_fps_from_milliseconds(times_ms: List[float]) -> float:
    """Return average FPS from a list of per-frame times in milliseconds."""
    if not times_ms:
        return 0.0
    avg_ms = sum(times_ms) / len(times_ms)
    if avg_ms == 0:
        return 0.0
    return 1000.0 / avg_ms


def ms_sum(speed_dict) -> float:
    """Helper: given a speed dict from Ultralytics `results[0].speed`,
    return total ms (preprocess + inference + postprocess). If keys missing,
    return 0.0.
    """
    try:
        return (
            float(speed_dict.get("preprocess", 0.0))
            + float(speed_dict.get("inference", 0.0))
            + float(speed_dict.get("postprocess", 0.0))
        )
    except Exception:
        return 0.0
