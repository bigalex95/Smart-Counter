#!/usr/bin/env python3
"""
SQLite Database Reader for Smart Counter
Demonstrates how to read and analyze the people counting data
"""

import sqlite3
import sys
import argparse
from pathlib import Path
from datetime import datetime

# Default database path
DEFAULT_DB_PATH = "logs/analytics.db"


def connect_db(db_path: str):
    """Connect to the SQLite database"""
    if not Path(db_path).exists():
        print(f"❌ Database not found: {db_path}")
        print("   Run the Smart Counter program first to create the database.")
        sys.exit(1)

    return sqlite3.connect(db_path)


def show_statistics(conn):
    """Show basic statistics from the database"""
    cursor = conn.cursor()

    # Get total records
    cursor.execute("SELECT COUNT(*) FROM people_count")
    total_records = cursor.fetchone()[0]

    if total_records == 0:
        print("ℹ️  Database is empty. No records found.")
        return

    print(f"📊 Database Statistics")
    print("=" * 50)
    print(f"Total records: {total_records}")
    print()

    # Get min, max, avg
    cursor.execute(
        """
        SELECT 
            MAX(in_count) as max_in,
            MAX(out_count) as max_out,
            AVG(in_count - out_count) as avg_occupancy
        FROM people_count
    """
    )
    max_in, max_out, avg_occupancy = cursor.fetchone()

    print(f"Total IN: {max_in}")
    print(f"Total OUT: {max_out}")
    print(f"Average Occupancy: {avg_occupancy:.2f}")
    print(f"Current Inside: {max_in - max_out if max_in and max_out else 0}")
    print()


def show_recent_records(conn, limit: int = 10):
    """Show the most recent records"""
    cursor = conn.cursor()
    cursor.execute(
        """
        SELECT id, timestamp, in_count, out_count
        FROM people_count 
        ORDER BY timestamp DESC 
        LIMIT ?
    """,
        (limit,),
    )

    records = cursor.fetchall()

    if not records:
        return

    print(f"📝 Last {limit} Records")
    print("=" * 70)
    print(f"{'ID':<6} {'Timestamp':<20} {'IN':<6} {'OUT':<6} {'INSIDE':<8}")
    print("-" * 70)

    for record_id, timestamp, in_count, out_count in records:
        inside = in_count - out_count
        print(
            f"{record_id:<6} {timestamp:<20} {in_count:<6} {out_count:<6} {inside:<8}"
        )
    print()


def show_daily_summary(conn):
    """Show summary grouped by date"""
    cursor = conn.cursor()
    cursor.execute(
        """
        SELECT 
            DATE(timestamp) as date,
            COUNT(*) as records,
            MAX(in_count) as total_in,
            MAX(out_count) as total_out,
            AVG(in_count - out_count) as avg_occupancy
        FROM people_count
        GROUP BY DATE(timestamp)
        ORDER BY date DESC
    """
    )

    records = cursor.fetchall()

    if not records:
        return

    print("📅 Daily Summary")
    print("=" * 80)
    print(
        f"{'Date':<12} {'Records':<10} {'Total IN':<10} {'Total OUT':<10} {'Avg Occupancy':<15}"
    )
    print("-" * 80)

    for date, record_count, total_in, total_out, avg_occupancy in records:
        print(
            f"{date:<12} {record_count:<10} {total_in:<10} {total_out:<10} {avg_occupancy:<15.2f}"
        )
    print()


def export_to_csv(conn, output_file: str = "export.csv"):
    """Export all data to CSV file"""
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM people_count ORDER BY timestamp")

    with open(output_file, "w") as f:
        # Write header
        f.write("id,timestamp,in_count,out_count,occupancy\n")

        # Write data
        for row in cursor.fetchall():
            occupancy = row[2] - row[3]  # in_count - out_count
            f.write(f"{row[0]},{row[1]},{row[2]},{row[3]},{occupancy}\n")

    print(f"✅ Data exported to: {output_file}")


def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description="Smart Counter - Database Reader",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--db",
        default=DEFAULT_DB_PATH,
        help=f"Path to SQLite database (default: {DEFAULT_DB_PATH})",
    )
    parser.add_argument(
        "--export",
        action="store_true",
        help="Automatically export data to CSV without prompting",
    )
    parser.add_argument(
        "--export-file",
        default="export.csv",
        help="CSV export filename (default: export.csv)",
    )

    args = parser.parse_args()

    print("🔍 Smart Counter - Database Reader")
    print()

    # Connect to database
    try:
        conn = connect_db(args.db)
        print(f"✅ Connected to database: {args.db}")
        print()
    except Exception as e:
        print(f"❌ Error connecting to database: {e}")
        sys.exit(1)

    try:
        # Show various statistics
        show_statistics(conn)
        show_recent_records(conn, limit=10)
        show_daily_summary(conn)

        # Export to CSV
        if args.export:
            export_to_csv(conn, args.export_file)
        else:
            export_choice = input("Export data to CSV? (y/n): ").lower()
            if export_choice == "y":
                export_to_csv(conn, args.export_file)

    except Exception as e:
        print(f"❌ Error: {e}")
    finally:
        conn.close()
        print("\n👋 Database connection closed.")


if __name__ == "__main__":
    # Check if pandas is available for enhanced functionality
    try:
        import pandas as pd

        print("✅ pandas available - enhanced functionality enabled")
        print()
    except ImportError:
        print("ℹ️  pandas not found - basic functionality only")
        print("   Install with: pip install pandas")
        print()

    main()
