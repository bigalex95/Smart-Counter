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
st.title("🚗 Smart Counter: Real-Time Analytics")


def load_data():
    """Читает данные из SQLite и возвращает DataFrame"""
    if not os.path.exists(DB_PATH):
        return pd.DataFrame()

    try:
        conn = sqlite3.connect(DB_PATH)
        # Читаем последние N записей (задается параметром --limit)
        query = f"SELECT timestamp, count FROM people_count ORDER BY timestamp DESC LIMIT {DATA_LIMIT}"
        df = pd.read_sql(query, conn)
        conn.close()

        # Конвертируем timestamp в datetime
        df["timestamp"] = pd.to_datetime(df["timestamp"])
        return df.sort_values("timestamp")
    except Exception as e:
        st.error(f"Error reading DB: {e}")
        return pd.DataFrame()


# Плейсхолдеры для метрик и графиков
metric_placeholder = st.empty()
chart_placeholder = st.empty()

# Автообновление каждые 2 секунды
while True:
    df = load_data()

    if not df.empty:
        # Вычисляем метрики
        current_count = df.iloc[-1]["count"]

        with metric_placeholder.container():
            col1, col2 = st.columns(2)
            col1.metric("Current People Count", current_count)
            col2.metric("Last Update", df.iloc[-1]["timestamp"].strftime("%H:%M:%S"))

        # Рисуем график
        with chart_placeholder.container():
            st.line_chart(df, x="timestamp", y="count")
    else:
        st.warning("Waiting for data...")

    time.sleep(REFRESH_INTERVAL)
