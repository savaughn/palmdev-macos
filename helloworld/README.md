# HelloWorld - Palm OS Application

A simple "Hello World" application demonstrating basic Palm OS development.

<img width="505" height="773" alt="Screenshot 2025-11-17 at 9 52 38 PM" src="https://github.com/user-attachments/assets/7b53aa71-5c47-49cb-a480-4343070e2f7e" />

## Features

- Displays a welcome message
- Shows an "About" dialog
- Demonstrates basic UI controls (buttons, labels)
- Includes menu handling
- Full event loop implementation

## Building

```bash
# Build the application
make build
```

Output: `HelloWorld.prc` ready to install on your Palm device.

## Installing

### Via USB/HotSync
```bash
make install
```

### Via Drag & Drop
- Open Palm Desktop or POSE emulator
- Drag `HelloWorld.prc` onto the device window

### Manual Installation
```bash
pilot-xfer -i HelloWorld.prc
```

## Modifying the Template

This is designed to be a starting point for your own applications:

1. **Copy the template**
   ```bash
   cd ..
   cp -r helloworld myapp
   cd myapp
   ```

2. **Update the app name**
   - Edit `Makefile`: change `APP_NAME` and `CREATOR_ID`
   - Rename source files to match
   - Update `#include` statements

3. **Design your UI**
   - Edit `.rcp` file to add forms, buttons, fields, etc.
   - Update `.h` file with new resource IDs
   - Implement handlers in `.c` file

4. **Build and test**
   ```bash
   make build
   ```

## File Structure

```
helloworld/
├── HelloWorld.c      # Main application code
├── HelloWorld.rcp    # UI resources (forms, alerts, menus)
├── Makefile          # Build configuration
└── README.md         # This file
```
