# Hướng dẫn bắt đầu trên Windows

Hướng dẫn dành cho VS Code + PowerShell. Backend có thể chạy và kiểm thử độc lập ngay cả khi Unreal Engine chưa được cài.

## 1. Clone hoặc cập nhật repository

Lần đầu:

~~~powershell
cd "$HOME\Desktop"
git clone https://github.com/conmeophat/FIFA-ONLINE-4.git
cd FIFA-ONLINE-4
code .
~~~

Nếu đã có repository:

~~~powershell
cd "$HOME\Desktop\FIFA-ONLINE-4"
git switch main
git pull origin main
code .
~~~

## 2. Tạo môi trường Python

Python 3.11 trở lên:

~~~powershell
py --version
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r backend\requirements.txt
~~~

Nếu PowerShell chặn kích hoạt môi trường trong cửa sổ hiện tại:

~~~powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\.venv\Scripts\Activate.ps1
~~~

## 3. Kiểm tra dữ liệu và chạy kiểm thử

Tại thư mục gốc repository:

~~~powershell
$env:PYTHONPATH = "backend"
python tools/validate_data.py
python -m pytest backend/tests -q
~~~

Các bản ghi hiện tại là dữ liệu giả lập để thử phần mềm. Không dùng chúng làm danh sách cầu thủ thật.

## 4. Khởi chạy API

Vẫn tại thư mục gốc:

~~~powershell
python -m uvicorn app.main:app --app-dir backend --reload --host 127.0.0.1 --port 8000
~~~

Mở các địa chỉ sau:

- http://127.0.0.1:8000/health — kiểm tra dịch vụ.
- http://127.0.0.1:8000/docs — giao diện thử API.
- http://127.0.0.1:8000/api/v1/clubs — danh sách CLB mẫu.
- http://127.0.0.1:8000/api/v1/leagues — danh sách giải mẫu.
- http://127.0.0.1:8000/api/v1/players — danh sách cầu thủ mẫu.

Dừng API bằng Ctrl+C.

## 5. Tạo Unreal project

1. Mở Epic Games Launcher → Unreal Engine → Library.
2. Cài bản Unreal Engine 5 tương thích với Windows và bộ công cụ C++.
3. Tạo project: Games → Blank → C++ → Desktop.
4. Đặt tên FootballGame và tạo trong thư mục unreal/.
5. Cài Visual Studio Build Tools với MSVC và Windows SDK theo yêu cầu của phiên bản Unreal.
6. Mở project một lần, sau đó đóng Unreal Editor.
7. Tại thư mục gốc repository, chạy:

~~~powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\tools\install_unreal_source.ps1
~~~

8. Regenerate project files và build target Development Editor trong Visual Studio.
9. Mở Unreal và tạo/lưu level mới.

## 6. Tạo sân và camera

Trong Unreal Editor:

1. Chọn Tools → Execute Python Script.
2. Chạy \`Content/Python/build_stadium_level.py\`.
3. Lưu level.

Hướng dẫn chi tiết: [STADIUM_BROADCAST_CAMERA.md](STADIUM_BROADCAST_CAMERA.md).

## 7. Tạo cầu thủ điều khiển được

1. Vẫn ở level sân bóng, chọn Tools → Execute Python Script.
2. Chạy \`Content/Python/build_player_test_level.py\`.
3. Nhấn Play.

Điều khiển mặc định:

| Phím / thiết bị | Chức năng |
|---|---|
| W / S | Tiến / lùi theo hướng camera |
| A / D | Sang trái / phải |
| Chuột | Xoay camera |
| Left Shift | Chạy nhanh khi giữ |
| Space | Nhảy thử nghiệm |
| Tay cầm – Left Stick | Di chuyển |
| Tay cầm – Right Stick | Xoay camera |
| Tay cầm – Left Shoulder | Chạy nhanh |

Cầu thủ ban đầu là mannequin blockout ghép từ mesh hình học có sẵn trong Unreal. Để dùng cầu thủ 3D có rig:
- Import Skeletal Mesh và Skeleton phù hợp.
- Tạo Animation Blueprint có parent class \`FootballPlayerAnimInstance\`.
- Gán Skeletal Mesh và Animation Blueprint vào actor \`FPO_Player_Test\`.

Hướng dẫn chi tiết: [PLAYER_ENGINE_V0_2.md](PLAYER_ENGINE_V0_2.md).

## 8. Quy tắc quan trọng

- Unreal C++/Blueprint chạy gameplay trong bản game đóng gói.
- Python Unreal chỉ dùng cho tự động hóa trong Editor.
- API HTTP dùng cho dữ liệu và nghiệp vụ, không dùng để mô phỏng vật lý bóng 60 lần/giây.
- Dữ liệu thật và asset thật chỉ được nhập khi xác định được nguồn, giấy phép và cách ghi nhận nguồn gốc.
