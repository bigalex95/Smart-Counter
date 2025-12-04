FROM python:3.9-slim

WORKDIR /app

# Install dependencies
RUN pip install --no-cache-dir streamlit pandas

# Copy application code
COPY app.py .

# Set default environment variables (can be overridden in docker-compose.yml)
ENV DB_PATH=/app/logs/analytics.db
ENV REFRESH_INTERVAL=2
ENV DATA_LIMIT=100

# Run Streamlit with CLI arguments from environment variables
CMD streamlit run app.py \
    --server.port=8501 \
    --server.address=0.0.0.0 \
    -- \
    --db "${DB_PATH}" \
    --refresh "${REFRESH_INTERVAL}" \
    --limit "${DATA_LIMIT}"