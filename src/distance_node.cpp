#include <chrono>
#include <memory>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include <gpiod.hpp>

using namespace std::chrono_literals;

class DistanceSensor : public rclcpp::Node {
    public:
        DistanceSensor() : Node("distance_sensor_node") {
            publisher_ = this->create_publisher<std_msgs::msg::Float32>("dist_topic", 10);

            // gpio setup
            chip = gpiod::chip("gpiochip4");
            trig_line = chip.get_line(23);
            trig_line.request({"trig", gpiod::line_request::DIRECTION_OUTPUT, 0});
            echo_line = chip.get_line(24);
            echo_line.request({"echo", gpiod::line_request::DIRECTION_INPUT, 0});

            timer_ = this->create_wall_timer(100ms, std::bind(&DistanceSensor::measure, this));
        }

    private:
        void measure() {
            trig_line.set_value(1);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            trig_line.set_value(0);

            while (echo_line.get_value() == 0);
            auto start = std::chrono::steady_clock::now();

            while (echo_line.get_value() == 1);
            auto end = std::chrono::steady_clock::now();

            std::chrono::duration<float> elapsed = end - start;
            float distance = (elapsed.count() * 34300) / 2;

            auto msg = std_msgs::msg::Float32();
            msg.data = distance;
            publisher_->publish(msg);
        }
        gpiod::chip chip;
        gpiod::line trig_line, echo_line;
        rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr publisher_;
        rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DistanceSensor>());
    rclcpp::shutdown();
    return 0;
}