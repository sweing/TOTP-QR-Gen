# TOTP-QR-Gen

TOTPify is a Python-based project for generating Time-based One-Time Password (TOTP) QR codes and validating them via a web interface.

## 🚀 Features
- 🔑 **TOTP Generation & Validation** using `pyotp`
- 🔒 **AES-GCM Encryption** to protect TOTP codes in transit
- 🏆 **Supports Multiple Users** with unique TOTP secrets
- 📡 **Flask API for Validation**
- 📱 **QR Code Generation** for easy scanning
- 🌐 **Works Offline** for generating TOTP QR codes

---

## 📂 Project Structure
```
totp_project/
│── .env                   # Stores TOTP secrets securely
│── .gitignore             # Ignore sensitive files
│── requirements.txt       # Python dependencies
│── app.py                 # Flask server for validation
│── generate_qr.py         # Generates encrypted QR codes
│── static/                # Stores generated QR codes
│   ├── alice123_totp_qr.png
│── README.md              # Project documentation
```

---

## 📦 Installation
1️⃣ **Clone the repository**

2️⃣ **Create a virtual environment (optional but recommended)**
```bash
python -m venv venv
source venv/bin/activate  # On Windows use `venv\Scripts\activate`
```

3️⃣ **Install dependencies**
```bash
pip install -r requirements.txt
```

---

## 🔧 Setup
1️⃣ **Create a `.env` file** and add user-specific TOTP secrets:
```bash
TOTP_SECRETS={"alice123": "JBSWY3DPEHPK3PXP", "bob456": "NB3WY3DPEHPK3QWE"}
```

2️⃣ **Generate QR Codes** for users:
```bash
python generate_qr.py
```
QR codes will be saved in the `static/` folder.

3️⃣ **Start the Flask server**
```bash
python app.py
```

---

## 🖼️ Usage
### ✅ **Step 1: Scan QR Code**
Use an **QR scanning app** to scan the QR code.

### ✅ **Step 2: Validate TOTP via API**
Send a request with the encrypted TOTP:
```
http://127.0.0.1:5000/validate?account_id=alice123&totp_enc=ENCRYPTED_TOTP_HERE
```
Response:
```json
{
  "success": true,
  "message": "TOTP is valid"
}
```

---

## 🔐 Security Measures
- ✅ **AES-GCM Encryption** ensures TOTP codes are never exposed in plaintext
- ✅ **Per-User Secrets** prevent unauthorized validation
- ✅ **Short-lived TOTP codes** reduce brute-force risks

---

## 🛠️ Future Improvements
- [ ] 🔄 **User management API** (Add/Remove users dynamically)
- [ ] 🖥️ **Web UI for scanning QR codes**
- [ ] 📡 **Docker support for easy deployment**

---

## 🤝 Contributing
Feel free to submit **issues, feature requests, or pull requests**! 🚀

---

## 📜 License
MIT License - Use it freely! 😊

