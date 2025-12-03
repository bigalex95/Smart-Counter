# Quick Start: Database Integration 🚀

## What Changed?

Your Smart Counter now saves people counting data to a SQLite database!

### Key Changes:

1. ✅ **Dockerfile** - Added `libsqlite3-dev` (already present) and `logs/` directory
2. ✅ **CMakeLists.txt** - Already configured with SQLite3 support
3. ✅ **database.h/cpp** - Already implemented database wrapper class
4. ✅ **main.cpp** - Updated to save data to `logs/analytics.db`
5. ✅ **deploy.sh** - Updated to mount `logs/` volume for persistence

---

## 🚀 Quick Start

### 1. Build the Docker Image

```bash
docker build -t smart-counter-cpp .
```

### 2. Run with Database Persistence

```bash
# Using deploy script (recommended)
./scripts/deploy.sh run --gpu

# Or manual Docker command
docker run -it --rm \
    --gpus all \
    -v $(pwd)/logs:/app/logs \
    smart-counter-cpp
```

**Important:** The `-v $(pwd)/logs:/app/logs` flag ensures your database persists on your host machine!

### 3. Check the Database

After running the program, check your database:

```bash
# Using the test script
./scripts/test_database.sh

# Or manually with sqlite3
sqlite3 logs/analytics.db "SELECT * FROM people_count LIMIT 10;"
```

---

## 📊 Database Structure

```sql
CREATE TABLE people_count (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    count INTEGER NOT NULL
);
```

**Example data:**

```
id  timestamp            count
1   2025-12-03 10:15:23  5
2   2025-12-03 10:15:45  7
3   2025-12-03 10:16:12  12
```

---

## 🔍 Viewing Your Data

### Option 1: SQLite CLI (Terminal)

```bash
# Open database
sqlite3 logs/analytics.db

# Run queries
sqlite> SELECT * FROM people_count ORDER BY timestamp DESC LIMIT 5;
sqlite> SELECT COUNT(*) FROM people_count;
sqlite> SELECT MAX(count) as max_people FROM people_count;
```

### Option 2: Python Script

```bash
# Run the provided Python reader
python3 python/read_database.py
```

This will show:

- 📊 Statistics (min, max, avg)
- 📝 Recent records
- 📅 Daily summary
- 💾 Option to export CSV

### Option 3: DB Browser for SQLite (GUI)

1. Download from: https://sqlitebrowser.org/
2. Open `logs/analytics.db`
3. Browse tables and run queries visually

---

## 💡 Key Features

### Smart Logging

- Only writes to DB when count **increases**
- Prevents 30+ writes per second
- Saves disk space and database performance

### Persistent Storage

- Data survives container restarts
- Lives on your host machine in `logs/`
- Easy to backup and analyze

### Production Ready

- ACID-compliant SQLite
- No separate database server needed
- Perfect for edge devices

---

## 📈 Example Queries

### Find peak times

```sql
SELECT
    strftime('%H:00', timestamp) as hour,
    MAX(count) as peak_count
FROM people_count
GROUP BY hour
ORDER BY peak_count DESC;
```

### Daily traffic

```sql
SELECT
    DATE(timestamp) as date,
    MAX(count) as max_count,
    AVG(count) as avg_count
FROM people_count
GROUP BY date;
```

### Recent activity

```sql
SELECT * FROM people_count
WHERE timestamp >= datetime('now', '-1 hour')
ORDER BY timestamp DESC;
```

---

## 🐛 Troubleshooting

### Database not created?

- Check that `logs/` directory exists
- Verify volume mount: `docker inspect <container> | grep Mounts -A 10`
- Check file permissions on `logs/` directory

### Database deleted after container stops?

- Make sure you're using the volume mount: `-v $(pwd)/logs:/app/logs`
- Don't use `--rm` if you want to inspect the container filesystem

### Can't write to database?

- Check logs for SQLite errors
- Verify `libsqlite3-dev` is installed in Docker image
- Ensure CMakeLists.txt links `SQLite::SQLite3`

---

## 📚 Learn More

- Full documentation: [docs/DATABASE.md](docs/DATABASE.md)
- Database schema: See `include/database.h`
- Integration code: See `src/main.cpp` (search for "Database")

---

## ✅ Verification Checklist

After your first run, verify:

- [ ] File `logs/analytics.db` exists on your host
- [ ] Running `./scripts/test_database.sh` shows data
- [ ] Can query database with `sqlite3 logs/analytics.db`
- [ ] Data persists after stopping container
- [ ] Python script can read the data

---

## 🎯 Next Steps

1. **Run the program** and let it collect some data
2. **Analyze the results** using the provided tools
3. **Export data** for visualization (CSV, Python, etc.)
4. **Build dashboards** with your favorite tools (Grafana, Plotly, etc.)

Enjoy your persistent analytics! 🎉
