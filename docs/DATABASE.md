# Database

The detector logs counter snapshots to a SQLite file (default `logs/analytics.db`, override with `--db` / `DB_PATH`). The file is created on first start, together with any missing parent directories (a bare file name such as `--db analytics.db` uses the current directory).

## Schema

```sql
CREATE TABLE IF NOT EXISTS people_count (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    in_count  INTEGER NOT NULL,   -- cumulative IN since the process started
    out_count INTEGER NOT NULL    -- cumulative OUT since the process started
);
```

`in_count` and `out_count` are running totals of the current run, not per-interval deltas. Occupancy is derived as `in_count - out_count`.

## When rows are written

A row is inserted only when `in_count + out_count` increases, i.e. once per counted crossing, not once per frame.

## Looking at the data

```bash
# sqlite3 CLI
sqlite3 logs/analytics.db "SELECT * FROM people_count ORDER BY id DESC LIMIT 10;"

# Summary / CSV export helper
python python/read_database.py --help

# Dashboard
streamlit run dashboard/app.py -- --db logs/analytics.db
```

The dashboard's "Reset Counters" button runs `DELETE FROM people_count`, so in Docker the `logs/` volume must be mounted read-write (the compose files do this).

## Notes

- `insert_log` builds the SQL string from two `int` values; the inputs are never user-supplied text. Prepared statements would be the cleaner choice.
- `Database` does not retry or surface errors beyond printing to stderr.
