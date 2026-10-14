-- Tạo cơ sở dữ liệu DATN_DATA_NhaXuong
CREATE DATABASE IF NOT EXISTS DATN_DATA_NhaXuong;

-- Sử dụng cơ sở dữ liệu vừa tạo
USE DATN_DATA_NhaXuong;

-- Tạo bảng nodes để lưu thông tin về các node trong hệ thống
CREATE TABLE nodes (
    node_id INT PRIMARY KEY,                 -- ID của node, là khóa chính để phân biệt các node
    name VARCHAR(50),                        -- Tên của node, có thể là tên thiết bị hoặc vị trí đặt node
    node_central TINYINT(1) DEFAULT 0          -- Cột để xác định xem node có phải là node trung tâm hay không (0: không, 1: có)
);

-- Tạo bảng sensor_data để lưu dữ liệu cảm biến từ các node
CREATE TABLE sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,              -- ID tự tăng để phân biệt các bản ghi
    node_id INT,                                    -- ID của node gửi dữ liệu
    time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- Thời gian ghi nhận dữ liệu, mặc định là thời điểm hiện tại
    cpu_temp FLOAT,                                 -- Nhiệt độ CPU
    temp FLOAT,                                     -- Nhiệt độ môi thiết bị
    current_val FLOAT,                              -- Dòng điện tiêu thụ
    vibration FLOAT,                                -- Độ rung của thiết bị   
    status TINYINT(1),                              -- Trạng thái hoạt động của thiết bị (0: bình thường, 1: cảnh báo)
    FOREIGN KEY (node_id) REFERENCES nodes(node_id) -- Liên kết với bảng nodes để đảm bảo tính toàn vẹn dữ liệu
);

-- Chèn dữ liệu mẫu vào bảng nodes
INSERT INTO nodes (node_id, name, node_central) VALUES (0, 'Node Trung Tâm', 1); 