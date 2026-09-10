Firebase: Control ESP32 GPIOs from Anywhere

This project allows you to control ESP32 GPIO pins remotely using Firebase Realtime Database. The ESP32 listens for changes in the database and updates outputs instantly.

Firebase Setup (Summary)

Create a Firebase project.
Disable Google Analytics.

Enable Authentication

Go to Authentication → Get Started
Enable Email/Password
Add a new user
Copy the user UID

Get Project API Key

Go to Project Settings
Copy the Web API Key

Create Realtime Database

Go to Realtime Database
Create Database
Choose your region
Copy the Database URL

Set Database Rules

Allow only your user (use your UID):

{
"rules": {
".read": "auth.uid === 'YOUR_USER_UID'",
".write": "auth.uid === 'YOUR_USER_UID'"
}
}

For multiple users:

{
"rules": {
".read": "auth.uid === 'UID1' || auth.uid === 'UID2'",
".write": "auth.uid === 'UID1' || auth.uid === 'UID2'"
}
}

Publish the rules.

Upload JSON Database Structure

Go to Realtime Database
Click the 3-dot menu → Import JSON
Select the provided JSON file

This will create:

board1
└── outputs
└── digital
├── 12 : 0
├── 13 : 0
└── 14 : 0

ESP32 Behavior

ESP32 signs in using email and password
Listens to "board1/outputs/digital/"
Updates GPIOs when database values change
Pins used: 12, 13, 14
On startup, ESP32 loads full JSON and syncs states

Testing

Change 0/1 values in Firebase Console
ESP32 updates instantly
Reset ESP32 → it syncs the latest GPIO states
