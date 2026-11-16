import streamlit as st
import sqlite3
import pandas as pd
import time
import os
import argparse
import sys


# Parse command-line arguments
def parse_args():
    """Parse command-line arguments"""
    parser = argparse.ArgumentParser(
        description="Smart Counter Real-Time Analytics Dashboard"
    )
    parser.add_argument(
        "--db",
        type=str,
        default=os.getenv("DB_PATH", "../logs/analytics.db"),
        help="Path to SQLite database (default: ../logs/analytics.db or DB_PATH env var)",
    )
    parser.add_argument(
        "--refresh",
        type=int,
        default=2,
        help="Refresh interval in seconds (default: 2)",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=100,
        help="Maximum number of records to display (default: 100)",
    )
    return parser.parse_args()


# Get command-line arguments
args = parse_args()
DB_PATH = args.db
REFRESH_INTERVAL = args.refresh
DATA_LIMIT = args.limit

st.set_page_config(page_title="Smart Counter Analytics", layout="wide")

# Header with reset button
col_title, col_reset = st.columns([4, 1])
with col_title:
    st.title("Smart Counter: People Counting Analytics")
with col_reset:
    st.write("")  # Spacer
    if st.button("🔄 Reset Counters", help="Clear all counter data from database"):
        try:
            if os.path.exists(DB_PATH):
                conn = sqlite3.connect(DB_PATH)
                cursor = conn.cursor()
                cursor.execute("DELETE FROM people_count")
                conn.commit()
                conn.close()
                st.success("✅ Counters reset successfully!")
                st.rerun()
        except Exception as e:
            st.error(f"❌ Error resetting counters: {e}")


def load_data():
    """Reads data from SQLite and returns a DataFrame"""
    if not os.path.exists(DB_PATH):
        return pd.DataFrame()

    try:
        conn = sqlite3.connect(DB_PATH)
        # Read the last N records (set by the --limit parameter)
        query = f"SELECT timestamp, in_count, out_count FROM people_count ORDER BY timestamp DESC LIMIT {DATA_LIMIT}"
        df = pd.read_sql(query, conn)
        conn.close()

        # Convert timestamp to datetime
        df["timestamp"] = pd.to_datetime(df["timestamp"])
        # Compute occupancy (how many are inside)
        df["occupancy"] = df["in_count"] - df["out_count"]
        return df.sort_values("timestamp")
    except Exception as e:
        st.error(f"Error reading DB: {e}")
        return pd.DataFrame()


# Placeholders for metrics and charts
metric_placeholder = st.empty()
chart_placeholder = st.empty()

# Auto-refresh every 2 seconds
while True:
    df = load_data()

    if not df.empty:
        # Compute metrics
        current_in = df.iloc[-1]["in_count"]
        current_out = df.iloc[-1]["out_count"]
        current_occupancy = df.iloc[-1]["occupancy"]

        # Drift protection: clamp negative occupancy values
        corrected_occupancy = max(0, current_occupancy)
        has_drift = current_occupancy < 0

        with metric_placeholder.container():
            col1, col2, col3, col4 = st.columns(4)
            col1.metric("👇 IN", current_in, delta=None, delta_color="normal")
            col2.metric("👆 OUT", current_out, delta=None, delta_color="inverse")

            # Show drift warning
            if has_drift:
                col3.metric(
                    "🏢 INSIDE",
                    f"{corrected_occupancy} ⚠️",
                    delta=f"Drift: {current_occupancy}",
                    delta_color="inverse",
                )
            else:
                col3.metric(
                    "🏢 INSIDE", corrected_occupancy, delta=None, delta_color="off"
                )

            col4.metric("⏰ Last Update", df.iloc[-1]["timestamp"].strftime("%H:%M:%S"))

            # Add drift warning
            if has_drift:
                st.warning(
                    f"⚠️ Tracker drift detected: Occupancy went negative ({current_occupancy}). "
                    f"This happens when people are counted on exit but missed on entry. "
                    f"Consider resetting counters or improving tracking conditions."
                )

        # Draw charts
        with chart_placeholder.container():
            st.subheader("📊 Crossing flow")

            # Create a DataFrame for a two-line chart
            chart_data = df[["timestamp", "in_count", "out_count", "occupancy"]].copy()
            chart_data = chart_data.set_index("timestamp")

            # Entries/exits chart
            st.line_chart(
                chart_data[["in_count", "out_count"]], color=["#00ff00", "#ff0000"]
            )

            st.subheader("👥 Occupancy Over Time")
            # Occupancy chart
            st.area_chart(chart_data[["occupancy"]], color=["#0088ff"])
    else:
        st.warning("Waiting for data...")

    time.sleep(REFRESH_INTERVAL)
