#!/bin/bash
# Script to test SQLite database functionality

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Default database path
DEFAULT_DB_PATH="${PROJECT_ROOT}/logs/analytics.db"
DB_PATH="${1:-$DEFAULT_DB_PATH}"

echo "🔍 Testing SQLite Database Integration"
echo "========================================"
echo ""

if [ "$DB_PATH" != "$DEFAULT_DB_PATH" ]; then
    echo "Using custom database path: $DB_PATH"
    echo ""
fi

# Check if sqlite3 is installed
if ! command -v sqlite3 &> /dev/null; then
    echo "❌ sqlite3 CLI not found. Install it with:"
    echo "   sudo apt install sqlite3"
    exit 1
fi

echo "✅ sqlite3 CLI found"
echo ""

# Create logs directory if it doesn't exist
mkdir -p "${PROJECT_ROOT}/logs"
echo "✅ Logs directory: ${PROJECT_ROOT}/logs"
echo ""

# Check if database exists
if [ -f "$DB_PATH" ]; then
    echo "📊 Database found: $DB_PATH"
    echo ""
    
    # Show table structure
    echo "📋 Table Structure:"
    echo "-------------------"
    sqlite3 "$DB_PATH" ".schema people_count"
    echo ""
    
    # Count records
    record_count=$(sqlite3 "$DB_PATH" "SELECT COUNT(*) FROM people_count;")
    echo "📈 Total records: $record_count"
    echo ""
    
    if [ "$record_count" -gt 0 ]; then
        # Show last 5 records
        echo "📝 Last 5 records:"
        echo "-----------------"
        sqlite3 -header -column "$DB_PATH" "SELECT * FROM people_count ORDER BY timestamp DESC LIMIT 5;"
        echo ""
        
        # Show statistics
        echo "📊 Statistics:"
        echo "-------------"
        sqlite3 -header -column "$DB_PATH" "
            SELECT 
                MIN(count) as min_count,
                MAX(count) as max_count,
                AVG(count) as avg_count,
                COUNT(*) as total_records
            FROM people_count;
        "
        echo ""
        
        # Show records by date
        echo "📅 Records by date:"
        echo "------------------"
        sqlite3 -header -column "$DB_PATH" "
            SELECT 
                DATE(timestamp) as date,
                COUNT(*) as records,
                MAX(count) as max_count,
                AVG(count) as avg_count
            FROM people_count
            GROUP BY DATE(timestamp)
            ORDER BY date DESC;
        "
        echo ""
    else
        echo "ℹ️  Database is empty. Run the program to collect data."
        echo ""
    fi
    
    # Export options
    echo "💾 Export Commands:"
    echo "------------------"
    echo "CSV export:  sqlite3 -header -csv $DB_PATH 'SELECT * FROM people_count;' > export.csv"
    echo "JSON export: sqlite3 $DB_PATH '.mode json' '.once export.json' 'SELECT * FROM people_count;'"
    echo ""
    
else
    echo "ℹ️  Database not found: $DB_PATH"
    echo "   The database will be created when you run the program."
    echo ""
    echo "🚀 To create and populate the database, run:"
    echo "   ./scripts/deploy.sh run --gpu"
    if [ "$DB_PATH" != "$DEFAULT_DB_PATH" ]; then
        echo "   with custom database: --db $DB_PATH"
    fi
    echo "   or"
    echo "   docker run -it --rm --gpus all -v \$(dirname $DB_PATH):/app/db_dir smart-counter-cpp --db /app/db_dir/\$(basename $DB_PATH)"
    echo ""
fi

# Show how to access database
echo "🔧 Database Access:"
echo "------------------"
echo "SQLite CLI:     sqlite3 $DB_PATH"
echo "Python:         python3 python/read_database.py --db $DB_PATH"
echo "DB Browser:     https://sqlitebrowser.org/"
echo ""
echo "Usage: $0 [database_path]"
echo "  Default: $DEFAULT_DB_PATH"
echo ""

echo "✅ Database test complete!"
