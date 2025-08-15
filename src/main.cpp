#include "watch/publisher/kafka_publisher.hpp"

int main(int argc, char *argv[])
{
    std::string broker = "192.168.1.23:9092";
    std::string topic = "mlp-logs";

    if (argc > 1)
        broker = argv[1];
    if (argc > 2)
        topic = argv[2];

    try
    {
        KafkaLogProducer producer(broker, topic);
        producer.sendLogBatches(5, 10); // 5批，每批10条
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
