# Hướng dẫn bắt đầu trên Windows

Hướng dẫn này dành cho VS Code + PowerShell. Backend có thể chạy và kiểm thử độc lập ngay cả khi Unreal Engine chưa được cài.

## 1. Clone repository

Mở PowerShell:

```powershell
cd "$HOME\Desktop"
git clone https://github.com/conmeophat/FIFA-ONLINE-4.git
cd FIFA-ONLINE-4
git switch develop
code .
```

## 2. Tạo môi trường Python

Python 3.11 trở lên:

```powershell
py --version
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install -r backend\requirements.txt
```

Nếu PowerShell chặn kích hoạt môi trường trong cửa sổ hiện tại:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\.venv\Scripts\Activate.ps1
```

## 3. Kiểm tra dữ liệu và chạy kiểm thử

Tại thư mục gốc repository:

```powershell
$env:PYTHONPATH = "backend"
python tools/validate_data.py
python -m pytest backend/tests -q
```

Các bản ghi hiện tại là dữ liệu giả lập để thử phần mềm. Không dùng chúng làm danh sách cầu thủ thật.

## 4. Khởi chạy API

Vẫn tại thư mục gốc:

```powershell
python -m uvicorn app.main:app --app-dir backend --reload --host 127.0.0.1 --port 8000
```

Mở các địa chỉ sau:

- `http://127.0.0.1:8000/health` — kiểm tra dịch vụ.
- `http://127.0.0.1:8000/docs` — giao diện thử API.
- `http://127.0.0.1:8000/api/v1/clubs` — danh sách CLB mẫu.
- `http://127.0.0.1:8000/api/v1/leagues` — danh sách giải mẫu.
- `http://127.0.0.1:8000/api/v1/players` — danh sách cầu thủ mẫu.

Ví dụ lọc:

```text
/api/v1/players?club_id=demo_blue_fc
/api/v1/players?position=ST
/api/v1/players?min_pace=80
```

Dừng API bằng `Ctrl+C`.

## 5. Tạo Unreal project

1. Mở Epic Games Launcher → Unreal Engine → Library.
2. Cài một bản Unreal Engine 5 tương thích với Windows và bộ công cụ C++.
3. Tạo project: Games → Blank → C++ → Desktop.
4. Đặt tên module/project là `FootballGame` và tạo trong thư mục `unreal/`.
5. Cài Visual Studio Build Tools với MSVC và Windows SDK theo yêu cầu của phiên bản Unreal.
6. Mở project một lần và build mặc định để xác nhận toolchain hoạt động.
7. Chép hai file trong `unreal/StarterSource/FootballGame/` vào thư mục `Source/FootballGame/` của project vừa tạo.
8. Thêm các module `HTTP`, `Json`, `JsonUtilities` vào `PublicDependencyModuleNames` trong `FootballGame.Build.cs`, sau đó regenerate project files và build.

Không commit các thư mục Unreal tự sinh như `Binaries`, `Intermediate`, `Saved` và `DerivedDataCache`.

## 6. Kiểm tra kết nối Unreal → Python

- Chạy backend trước trên cùng máy tính.
- Mở Unreal project và gọi hàm `RequestClubs` từ Blueprint thông qua `FootballApiSubsystem`.
- Đăng ký hai sự kiện `OnClubsReceived` và `OnRequestFailed`.
- JSON nhận được là danh mục đội bóng mẫu.
- Không gọi HTTP mỗi frame trong `Tick`; chỉ yêu cầu dữ liệu lúc mở menu/đội hình hoặc lúc cần lưu/tải.

## 7. Quy tắc quan trọng

- Unreal C++/Blueprint chạy gameplay trong bản game đóng gói.
- Python Unreal chỉ dùng cho tự động hóa trong Editor.
- API HTTP dùng cho dữ liệu và nghiệp vụ, không dùng để mô phỏng vật lý bóng 60 lần/giây.
- Dữ liệu thật chỉ được nhập khi xác định được nguồn, giấy phép và cách ghi nhận nguồn gốc.
