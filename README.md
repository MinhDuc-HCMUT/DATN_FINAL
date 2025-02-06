# THIẾT KẾ VÀ TRIỂN KHAI HỆ THỐNG KHÓA CỬA THÔNG MINH TÍCH HỢP CÔNG NGHỆ RFID VÀ PHƯƠNG THỨC BẢO MẬT SINH TRẮC HỌC

## 1. Tổng quan hệ thống
![image](https://github.com/user-attachments/assets/af4f9688-2118-4ac2-9a1b-598ddb610fbb)

Hệ thống được xây dựng thành 4 lớp:
-	**Perception Layer:** chứa Device (thiết bị phần cứng). Người dùng tương tác với phần cứng thông qua các lớp bảo mật thẻ từ RFID, vân tay, gương mặt và mật khẩu. Tại đây dữ liệu thông tin về hành vi sử dụng hệ thống sẽ được thu thập và gửi đến lớp tiếp theo.
-	**Network Layer:** sử dụng giao thức truyền thông Serial COM Port (giao tiếp nối tiếp) để gửi dữ liệu thu thập từ thiết bị cho các lớp phía trên.
-	**Processing Layer:** chứa server-backend (xây dựng với Flask framework và database SQLite) có vai trò nhận dữ liệu từ device xử lý, tính toán, sau cùng sẽ đẩy lên web browser.
-	**Application Layer:** chứa web browser, có khả năng nhận dữ liệu từ backend, hiển thị trực quan tới người dùng. Người dùng có thể theo dõi, tinh chỉnh thông tin hiển thị theo ý muốn.

## 2. Yêu cầu chức năng của hệ thống
![image](https://github.com/user-attachments/assets/388e7810-2fa4-4224-a14c-50fca8cff7c7)

## 3. Sơ đồ khối của hệ thống
![image](https://github.com/user-attachments/assets/dec3c6a1-f48d-4bcd-b78b-125f9f64a0ed)

## 4. Chương trình nhận diện gương mặt tương tác với hệ thống
https://github.com/MinhDuc-HCMUT/FACE_RECOGNITION_DATN.git 
