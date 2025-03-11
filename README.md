# TOTP-QR-Gen

TOTP-QR-Gen is an open-source project that combines a Flask server with an ESP32 microcontroller to provide proof of presence through Time-based One-Time Passwords (TOTP) displayed as QR codes. The project aims to create a secure and decentralized method to verify physical presence using dynamically generated QR codes.

## Features
- TOTP Generation & Validation using `pyotp`
- AES-ECB Encryption to protect TOTP codes in transit
- ESP32 Microcontroller to generate and display QR codes offline
- Supports Multiple Users with unique TOTP secrets
- Flask API for Validation
- QR Code Generation for easy scanning
- Works Offline for generating TOTP QR codes
- Admin Dashboard for monitoring TOTP validation attempts

---

## Project Structure
```
TOTP-QR-Gen/
│── app.py                 # Flask server for TOTP validation
│── totp_generator.py      # Generates encrypted QR codes
│── requirements.txt       # Python dependencies
│── .env                   # Stores TOTP secrets and config
│── static/                # Stores generated QR codes and admin logs
│   ├── alice123_totp_qr.png
│── templates/             # HTML templates for Flask
│   ├── admin.html
│── esp32/                 # Code for the ESP32 microcontroller
│   ├── esp32_code.ino
│── README.md              # Project documentation
|── post.md                # Blog post
```

---

## Installation
1. Clone the repository

2. Create a virtual environment (optional but recommended)
```bash
python -m venv venv
source venv/bin/activate  # On Windows use `venv\Scripts\activate`
```

3. Install dependencies
```bash
pip install -r requirements.txt
```

---

## Setup
1. Create a `.env` file and add user-specific TOTP secrets and validation settings:
```bash
TOTP_SECRETS={"alice123": "JBSWY3DPEHPK3PXP", "bob456": "NB3WY3DPEHPK3QWE"}
MAX_VALIDATIONS=3  # Number of times a single TOTP can be validated
```

2. Start the Flask server
```bash
python app.py
```

3. Generate QR Codes for users:
```bash
python totp_generator.py
```
QR codes will be saved in the `static/` folder.

---

## ESP32 Setup

### Prerequisites
- Arduino IDE with ESP32 Board Manager installed
- Required Libraries: Adafruit_GFX, GxEPD2, WiFi, Time, TOTP, QRCode

### Uploading Code

- Connect your ESP32 to the computer.
- Open esp32/esp32_code.ino in the Arduino IDE.
- Compile and upload the code to the ESP32.

### Hardware Components

- **ESP32 Microcontroller**: Upgrade from the ESP8266 because I needed more GPIO pins.
- **NEO-6M GPS Module**: To acquire location and time data.
- **DS3231 Real-Time Clock (RTC) Module**: To maintain accurate time.
- **Waveshare 1.54" Black/White E-Paper Display**: I chose a black-and-white display over a color one because the refresh rate is much faster (2 seconds vs. 8 seconds).
- **2N2222 NPN Bipolar Transistor**: To switch off the GPS module once it acquires the location and time.
- **Breadboard and Cables**: For prototyping.
- **5.1K Resistor**

---

## Encryption Method
The TOTP codes are encrypted using **AES-ECB encryption**. 

The key is padded or truncated to 32 bytes, and the plaintext is padded to match AES's block size before encryption.

---

## Usage
### Step 1: Scan QR Code
The ESP32 device (or the pyhton script `totp_generator.py`) generates and displays a QR code with an encrypted TOTP.

### Step 2: Validate TOTP via API
Use a QR scanner to submit the code to the server:
```
http://127.0.0.1:5000/validate?account_id=alice123&totp_enc=ENCRYPTED_TOTP_HERE
```
Example Response:
```json
{
  "success": true,
  "message": "TOTP is valid"
}
```

---

## Admin Dashboard
The Admin Dashboard provides real-time logs of both successful and failed validation attempts, including:
- Timestamp of the attempt
- Account ID (if provided)
- Validation Status (Success/Failed)
- Failure Reason (e.g., invalid TOTP, exceeded max attempts)
- IP Address of the requester

### Access the Admin Dashboard
Once the Flask server is running, open your browser and visit:
```
http://127.0.0.1:5000/admin
```
This will display a log of all validation attempts.

---

## Future Improvements
- User management API (Add/Remove users dynamically)
- Web UI for scanning QR codes
- Docker support for easy deployment
- Admin Authentication for secure access to logs

---

## Contributing
Feel free to submit issues, feature requests, or pull requests!

---

## License
MIT License - Use it freely!