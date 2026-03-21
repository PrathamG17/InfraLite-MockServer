# InfraLite-MockServer

A lightweight, cross-platform mock HTTP server designed for API testing and development. Built with C++ and Qt, 
InfraLite-MockServer provides QA teams, frontend developers, and mobile developers with a powerful tool to simulate 
APIs, test edge cases, and develop against stable mock endpoints.

## 🎯 Project Overview

InfraLite-MockServer is a desktop application that allows teams to:
- Create and manage mock HTTP endpoints
- Simulate errors and edge cases for testing
- Serve static files and dynamic responses
- Monitor and export request/response logs
- Configure server behavior without code changes

## 🏗️ Architecture [STILL UNDER DEVELOPMENT]

The project follows a modular architecture with three main components:

### 1. Server Module (`src/server/`)
Core HTTP server implementation handling requests, routing, and responses.

**Key Classes:**
- `CServer` - Main server class managing socket connections
- `Router` - Routes requests to appropriate handlers
- `HttpRequest` / `HttpResponse` - HTTP message abstractions
- `FileHandler` - Serves static files
- `ConfigLoader` - Loads route configurations
- `Logger` - Centralized logging system

### 2. Database Module (`src/db/`)
Data persistence layer using SQLite for storing routes, users, logs, and configurations.

**Key Classes:**
- `DatabaseHandler` - Core database connection manager
- `UserRepository` - User CRUD operations
- `RouteRepository` - Mock route management
- `FileRepository` - Static file metadata storage
- `LogRepository` - Access log persistence
- `ConfigRepository` - Server configuration storage

**Database Schema:**
- `USER` - User accounts with role-based access
- `MOCKROUTE` - Mock endpoint definitions
- `STATICFILE` - File metadata for static responses
- `ACCESSLOG` - Request/response audit trail
- `SERVERCONFIG` - Key-value configuration store

### 3. UI Module (`src/ui/`)
Qt-based desktop interface providing role-specific workflows.

**Key Windows:**
- `LoginWindow` - User authentication
- `DashboardWindow` - Server status and statistics
- `RouteMgmtWindow` - Create/edit mock routes
- `FileMgmtWindow` - Upload and manage static files
- `ConfigWindow` - Server configuration
- `LogWindow` - Live log monitoring
- `ErrorSimWindow` - Error simulation for testing
- `UserMgmtWindow` - User administration

### Prerequisites

- **C++ Compiler**: GCC 9+ or Clang 10+ (C++17 support required)
- **Qt Framework**: Qt 5.15+ or Qt 6.x
- **CMake**: 3.16 or higher
- **SQLite3**: 3.30 or higher
- **Git**: For version control

###  Build Instructions

1. Clone the repository
```bash
git clone https://github.com/your-org/InfraLite-MockServer.git
cd InfraLite-MockServer
```

2. Set up vcpkg
```bash
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git

# Bootstrap vcpkg
.\vcpkg\bootstrap-vcpkg.bat

# Install required dependencies (example: OpenSSL)
.\vcpkg install openssl:x64-windows-static
```

3. Configure with CMake presets
```bash
cmake --preset x64-debug
```

4. Build the project
```bash
cmake --build --preset x64-debug
```

5. Run the application
Before running, set the JWT secret environment variable (Windows PowerShell):

```powershell
$env:JWT_SECRET="myLocalSecret"
```

Then execute:
```bash
.\build\x64-debug\InfraLite-MockServer.exe
```

## 📁 Project Structure

```
InfraLite-MockServer/
|── include/
|  |──server            -------- (For server .hpp)
|  |──db                -------- (For database .hpp)
|  |──ui                -------- (For UI classes header files .hpp)
|  |──nlohmann          -------- (Other Externally Downloaded .h/.hpp  mention alnogside server, db and ui)
├── src/
│   ├── server/          ------- (HTTP server implementation [Still Under Development])
│   │   ├── CServer.cpp
│   │   ├── Router.cpp
│   │   ├── HttpRequest.cpp
│   │   ├── HttpResponse.cpp
│   │   ├── FileHandler.cpp
│   │   ├── ConfigLoader.cpp
│   │   └── Logger.cpp/h
│   ├── db/              -------- (Database layer [Yet To Start])
│   └── ui/              -------- (Qt UI components [Yet To Start])
├── static/              -------- (For Testing Static File Serving Capability Of Server Via Defined Routes, able to test via browser as (eg. : localhost:8080/index.html))
|   ├── css/
|   |   └── style.css 
|   ├── data/
|   |   └── sample.json 
|   ├── images/
|   |   ├── banner.jpg
|   |   └──  logo.png
|   ├── js/
|   |   └── app.js
|   └── index.html
├── tests/                 -------- (python test script [not for unit level])
├── config/                -------- (routes uploaded to the system via files, that files were stored here...)
|── logs/
|   └── InfraLite-MockServer.log     ----------------(All Server Logs Stored Here For Debugging Purpose Whether it's running or stop etc.)
├── CMakeLists.txt         -------- (Build configuration)
├── .gitignore             -------- (Files which never wanted to push over github project mentioned here..eg.rough files)
├── .github/
|   ├── workflows/
|   |   ├── ci.yml         -------- (continous integration file responsible for configure,build and run mention test scripts while push and opening PR to dvelop/main branch )
└── README.md              -------- (Read Carefully Before Starting With Project.)
```

## 🧪 Testing
1) For Server Testing =>
   --> I added, test_routes.py in tests folder. After succesffully running the server then execute test_routes
       file as (python -m pytest tests\test_routes.py -v) you found that, mentioned routes were tested and result
       as Passed. Same test_routes.py file also got execute when we create a pull request or push code in develop
       branch as a result of continous integration via ci.yml file in .github/workflows directory.

    Note: I Appeal you'd also create your module test scripts as test_db, test_ui a whenever implementation is
          at the stage of testing and add it to tests, so that if it's run successfully then you'd add it to
          ci.yml after discussion so that other than that module developer also get benefited.
   
## 📊 Database Schema

The database follows 3NF normalization:

- **USER**: User accounts with role-based access control
- **MOCKROUTE**: Mock endpoint definitions with method/path/response
- **STATICFILE**: File metadata linked to routes (CASCADE delete)
- **ACCESSLOG**: Request/response audit logs with indexing
- **SERVERCONFIG**: Key-value configuration store

## 🤝 For Team Members
Follow Setps =>
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
5. Push to the branch (`git push origin feature/AmazingFeature`)
6. Open a Pull Request

## 👨‍💻 Team

- **Server Module**: [@PrathamG17, @ShreyasChothe]
- **Database Module**: [@ShreyasChothe]
- **UI Module**: [@cs-pss, @ShreyasChothe, @PrathamG17]

## 🐛 Known Issues

--> Track bugs and feature requests in [GitHub Issues](https://github.com/yourusername/InfraLite-MockServer/issues).

## 📚 Documentation

--> Still No Documentation references here, i feel we'll made as per our module were built step by step, so that anyone within us can directly call respective member function from other module class while 
performing integration when base functionality is implemented & tested.
