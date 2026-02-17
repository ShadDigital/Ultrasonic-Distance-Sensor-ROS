#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include <gpiod.hpp>

class LedController : public rclcpp::Node {
    public:
        LedController() : Node("led_controller_node") {
            auto chip = gpiod::chip("gpiochip4");
            led_line = chip.get_line(18);
            led_line.request({"led", gpiod::line_request::DIRECTION_OUTPUT, 0});

            subscription_ = this->create_subscription<std_msgs::msg::Float32>(
                "dist_topic", 10, std::bind(&LedController::callback, this, std::placeholders::_1)
            );
        }
    
        private:
            void callback(const std_msgs::msg::Float32::SharedPtr msg) {
                if (msg->data < 20.0) {
                    led_line.set_value(1);
                    RCLCPP_INFO(this->get_logger(), "Object detected at: %f cm", msg->data);
                } else {
                    led_line.set_value(0);
                }
            }
            gpiod::line led_line;
            rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr subscription_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LedController>());
    rclcpp::shutdown();
    return 0;
}