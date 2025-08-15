import json
import random
import time

from kafka import KafkaProducer
from loguru import logger

producer = KafkaProducer(
    bootstrap_servers=["192.168.1.23:9092"],  # Specifies the Kafka server to connect
    value_serializer=lambda x: json.dumps(x).encode(
        "utf-8"
    ),  # Serializes data as JSON and encodes it to UTF-8 before sending
    batch_size=16384,  # Sets the maximum batch size in bytes (here, 16 KB) for buffered messages before sending
    linger_ms=10,  # Sets the maximum delay (in milliseconds) before sending the batch
    acks="all",  # Specifies acknowledgment level; 'all' ensures message durability by waiting for all replicas to acknowledge
)


def generate_log_message():
    levels = ["INFO", "WARNING", "ERROR", "DEBUG"]
    messages = [
        "User login successful",
        "User login failed",
        "Database connection established",
        "Database connection failed",
        "Service started",
        "Service stopped",
        "Payment processed",
        "Payment failed",
    ]
    log_entry = {
        "level": random.choice(levels),
        "message": random.choice(messages),
        "timestamp": int(time.time() * 1000),
    }
    return log_entry


def send_log_batches(topic, num_batches=5, batch_size=10):
    for i in range(num_batches):
        logger.info(f"Sending batch {i + 1}/{num_batches}")
        for _ in range(batch_size):
            log_message = generate_log_message()
            producer.send(topic, value=log_message)
        producer.flush()


if __name__ == "__main__":
    topic = "mlp-logs"
    send_log_batches(topic)
    producer.close()
