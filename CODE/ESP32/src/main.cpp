#include <ArduinoJson.h> // Thư viện để làm việc với JSON
#include <PubSubClient.h> // Thư viện MQTT client
#include <WiFi.h> // Thư viện WiFi
#include <WiFiManager.h> // Thư viện để cấu hình WiFi dễ dàng
#include <HardwareSerial.h> // Thư viện để sử dụng Serial phần cứng (UART)

// Định nghĩa thông tin MQTT server và các topic
const char* mqtt_server = "172.20.10.8"; // Địa chỉ IP của MQTT Broker
const char* topic_publish = "haui/DA_TN/data"; // Topic để ESP32 publish dữ liệu lên
const char* topic_subscribe = "haui/DA_TN/commands"; // Topic để ESP32 đăng ký nhận lệnh từ Node-RED

WiFiClient espClient; // Đối tượng WiFiClient để ESP32 kết nối qua WiFi
PubSubClient client(espClient); // Đối tượng PubSubClient để ESP32 giao tiếp MQTT

HardwareSerial MySerial(2); // Khai báo đối tượng Serial phần cứng thứ 2 (UART2)

// Hàm này được gọi khi kết nối MQTT bị mất hoặc chưa được thiết lập
void reconnect() {
  while (!client.connected()) { // Lặp lại cho đến khi kết nối MQTT thành công
    Serial.print("Đang kết nối MQTT..."); // In trạng thái kết nối MQTT ra Serial Monitor
    if (client.connect("ESP32_Gateway")) { // Cố gắng kết nối MQTT với ID "ESP32_Gateway"
      Serial.println("đã kết nối"); // In thông báo kết nối thành công
      client.subscribe(topic_subscribe); // Đăng ký nhận tin nhắn từ topic_subscribe
      delay(200); // Đợi một chút
      // Gửi yêu cầu lấy thông tin nhiệt độ/độ ẩm tới Node-RED (ví dụ)
      client.publish("haui/DA_TN/requests", "{\"type\":\"GET_TH\"}");
    } else {
      Serial.print("kết nối thất bại, mã lỗi="); // In thông báo kết nối thất bại
      Serial.print(client.state()); // In mã lỗi MQTT
      delay(5000); // Đợi 5 giây trước khi thử lại
    }
  }
}

// Hàm callback này được gọi khi có tin nhắn MQTT đến từ server
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Tin nhắn mới từ Node-Red ["); // In thông báo nhận tin nhắn
  Serial.print(topic); // In topic của tin nhắn
  Serial.print(": ");
  String message; // Khai báo biến String để lưu nội dung tin nhắn
  for (int i = 0; i < length; i++) {
    message += (char)payload[i]; // Chuyển đổi payload (byte array) thành String
  }
  Serial.println(message); // In nội dung tin nhắn ra Serial Monitor
  MySerial.println(message); // Gửi nội dung tin nhắn qua Serial2 (đến STM32)
}

void setup() {
  Serial.begin(115200, SERIAL_8N1); // Khởi tạo Serial Monitor với tốc độ baud 115200
  MySerial.begin(9600, SERIAL_8N1, 19, 18); // Khởi tạo Serial2 (UART2) với tốc độ baud 9600, chân RX: 19, chân TX: 18

  WiFiManager wm; // Tạo đối tượng WiFiManager
  // wm.resetSettings(); // Bỏ ghi chú dòng này để xóa thông tin WiFi đã lưu

  // Tự động kết nối WiFi, nếu chưa có thông tin sẽ tạo AP "ESP32_Config" để cấu hình
  bool res = wm.autoConnect("ESP32_Config");
  if(!res) {
    Serial.println("Kết nối WiFi thất bại!"); // Báo lỗi nếu kết nối WiFi không thành công
  } else {
    Serial.println("Kết nối WiFi thành công!"); // Báo thành công nếu kết nối WiFi OK
  }

  client.setServer(mqtt_server, 1883); // Thiết lập địa chỉ MQTT Broker và cổng (mặc định là 1883)
  client.setCallback(callback); // Gán hàm callback để xử lý tin nhắn MQTT đến
}

void loop() {
  if (!client.connected()) { // Kiểm tra xem ESP32 có đang kết nối MQTT không
    reconnect(); // Nếu không, gọi hàm reconnect để kết nối lại
  }
  client.loop(); // Xử lý các tác vụ MQTT định kỳ (nhận/gửi tin nhắn)

  // Kiểm tra xem có dữ liệu đến từ Serial2 (từ STM32) không
  if (MySerial.available() > 0) {
    String data = MySerial.readStringUntil('\n'); // Đọc dữ liệu từ Serial2 cho đến khi gặp ký tự xuống dòng
    data.trim(); // Loại bỏ khoảng trắng thừa ở đầu và cuối chuỗi

    if (data.length() > 0) { // Nếu có dữ liệu nhận được
      Serial.print("Dữ liệu nhận từ STM32: ");
      Serial.println(data); // In dữ liệu nhận được ra Serial Monitor

      JsonDocument doc; // Tạo một đối tượng JSON Document
      DeserializationError error = deserializeJson(doc, data); // Phân tích chuỗi JSON nhận được

      if (error) { // Nếu có lỗi khi phân tích JSON
        Serial.print("Lỗi khi phân tích JSON: ");
        Serial.println(error.f_str()); // In thông báo lỗi
        return; // Thoát khỏi hàm loop
      }

      doc["rssi"] = WiFi.RSSI(); // Thêm trường "rssi" (cường độ tín hiệu WiFi) vào JSON
      doc["online"] = true; // Thêm trường "online" với giá trị true vào JSON

      char buffer[512]; // Tạo một buffer để chứa chuỗi JSON đã serialize
      serializeJson(doc, buffer); // Chuyển đổi đối tượng JSON thành chuỗi và lưu vào buffer

      if(client.publish(topic_publish, buffer)) // Gửi chuỗi JSON lên topic_publish
      {
        Serial.println("Đã gửi!"); // Thông báo gửi thành công
      }
      else
      {
        Serial.println("Gửi tin nhắn thất bại"); // Thông báo gửi thất bại
      }
    }
  }
}

