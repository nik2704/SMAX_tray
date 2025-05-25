# 🛎️ SMAX Tray Client

**SMAX Tray Client** is a lightweight Windows application that runs in the system tray and periodically monitors a request queue.
Requests are selected by the criteria: should be Active and OwnedByPerson.Upn equals User Name.
When new requests appear, it notifies the user with a system tray notification.

---

## 📦 Project Structure

```
smax_tray_client/
├── icons/
│   ├── RequestInitial.ico
│   ├── RequestNewItems.ico
│   └── Error.ico
├── Utils
|   ├── Utils.cpp
|   ├── Utils.h
├── Checker/
|   ├── ConfigManager
|   |   ├── ConfigInitializer
|   |   |   ├── ConfigInitializer.h
|   |   |   ├── ConfigInitializer.cpp
|   |   |   └── ConfigInitializerDialog.rc
|   ├── JSONhelper
|   |   ├── CMakeLists.txt
|   |   ├── json_builder.h
|   |   ├── json_builder.cpp
|   |   ├── json.h
|   |   ├── json.cpp
|   |   ├── JSONhelper.h
|   |   └── JSONhelper.cpp
|   ├── NetworkClient
|   |   ├── NetworkClient.h
|   |   └── NetworkClient.cpp
|   ├── TrayManager
|   |   ├── TrayManager.h
|   |   └── TrayManager.cpp
|   ├── Worker
|   |   ├── Worker.h
|   |   └── Worker.cpp
|   ├── Checker.h
|   ├── Checker.cpp
|   ├── Requestor.h
|   └── Requestor.cpp
├── CMakeLists.txt
├── main.cpp
├── config.ini
├── resource.h
└──  resources.rc
```
---

## ✨ Features

- 🖥️ Runs silently in the Windows system tray.
- 🔒 Creates an initial INI file in a dialog mode if there is no the INI file.
- ⚙️ Allows to change settings in a dialog mode.
- 🔒 Checks the value of a token for an instance and in case of '-init' requests this information and encrypts the token.
- 🌐 Periodically polls a REST API for new service requests.
- 🔔 Displays Windows notifications for new or unacknowledged items.
- 🔒 Authenticates via username and token.
- ⚙️ Fully configurable via `config.ini`.

---

## ⚙️ Configuration: `config.ini`

Place this file in the same directory as the executable. Example:

```ini
[Settings]
instance=SHARED
period=60

[SHARED]
hostname=hostname_shared
tenantId=actual_tenant_id_shared
token=-init-
; actual_personal_access_encrypted_token_customer1. init => request user to provide the token

[CUSTOMER1]
hostname=hostname_customer1
tenantId=actual_tenant_id_customer1
userName=actual_username_customer1
Stoken=-init-
; actual_personal_access_encrypted_token_customer1. init => request user to provide the token

```

---

## 🧰 Requirements

- Windows OS
- CMake ≥ 3.12
- C++17-compatible compiler

---

## 🛠️ Building the Project

1. Install Boost and set the `BOOST_ROOT` path correctly.
2. Run the following commands in a terminal:

```bash
cmake -S . -B build
cmake --build build
cmake --build build --config Release
```

This generates the executable `smax_tray_client.exe`.

---

## ▶️ Running

Double-click the executable to run. The application will appear as an icon in the Windows system tray.

Click the tray icon to acknowledge and clear notifications.

---

## 📤 Notifications

- When new items are detected in the request queue, the app will:
  - Change the tray icon to `RequestNewItems.ico`.
  - Show a Windows balloon notification with request details.

- Acknowledging the notification will:
  - Reset the tray icon to `RequestInitial.ico`.
  - Opens the request queue in a browser.
  - Suppress further alerts until new items arrive.

---

## 📄 License

This project is released under the MIT License. See `LICENSE` for details.

---

## 👤 Author

Created by [Nikolay]. For questions or feedback, contact: [nik2704@yandex.ru].

