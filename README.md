Here's your updated `README.md` with the new **Admin Dashboard** functionality included:

---

# TOTP-QR-Gen

TOTP-QR-Gen is a Python-based project for generating Time-based One-Time Password (TOTP) QR codes and validating them via a web interface.

## 🚀 Features
- 🔑 **TOTP Generation & Validation** using `pyotp`
- 🔒 **AES-GCM Encryption** to protect TOTP codes in transit
- 🏆 **Supports Multiple Users** with unique TOTP secrets
- 📡 **Flask API for Validation**
- 📱 **QR Code Generation** for easy scanning
- 🌐 **Works Offline** for generating TOTP QR codes
- 📊 **Admin Dashboard** for monitoring TOTP validation attempts

---

## 📂 Project Structure
```
TOTP-QR-Gen/
│── .env                   # Stores TOTP secrets securely
│── .gitignore             # Ignore sensitive files
│── requirements.txt       # Python dependencies
│── app.py                 # Flask server for validation & admin dashboard
│── totp_generator.py      # Generates encrypted QR codes
│── templates/             # HTML templates for admin dashboard
│   ├── admin.html         # Admin dashboard template
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
1️⃣ **Create a `.env` file** and add user-specific TOTP secrets and validation settings:
```bash
TOTP_SECRETS={"alice123": "JBSWY3DPEHPK3PXP", "bob456": "NB3WY3DPEHPK3QWE"}
MAX_VALIDATIONS=3  # Number of times a single TOTP can be validated
```

2️⃣ **Start the Flask server**
```bash
python app.py
```

3️⃣ **Generate QR Codes** for users:
```bash
python totp_generator.py
```
QR codes will be saved in the `static/` folder.

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

## 📊 Admin Dashboard
The Admin Dashboard provides real-time logs of both **successful** and **failed** validation attempts, including:
- 🕒 **Timestamp** of the attempt  
- 🆔 **Account ID** (if provided)  
- ✅/❌ **Validation Status** (Success/Failed)  
- ❌ **Failure Reason** (e.g., invalid TOTP, exceeded max attempts)  
- 🌍 **IP Address** of the requester  

### 🔍 Access the Admin Dashboard
Once the Flask server is running, open your browser and visit:
```
http://127.0.0.1:5000/admin
```
This will display a **log of all validation attempts**.

---

## 🔐 Security Measures
- ✅ **AES-GCM Encryption** ensures TOTP codes are never exposed in plaintext
- ✅ **Per-User Secrets** prevent unauthorized validation
- ✅ **Short-lived TOTP codes** reduce brute-force risks
- ✅ **Max Validation Limits** prevent excessive reuse of TOTP codes
- ✅ **Admin Dashboard Logs** track all validation attempts for security monitoring

---

## 🛠️ Future Improvements
- [ ] 🔄 **User management API** (Add/Remove users dynamically)
- [ ] 🖥️ **Web UI for scanning QR codes**
- [ ] 📡 **Docker support for easy deployment**
- [ ] 🔐 **Admin Authentication** for secure access to logs

---

## 🤝 Contributing
Feel free to submit **issues, feature requests, or pull requests**! 🚀

---

## 📜 License
MIT License - Use it freely! 😊

---

### ✅ **Changes Added in This Update**
- 🆕 **Admin Dashboard** for tracking all validation attempts  
- 🆕 **MAX_VALIDATIONS** setting to limit how many times a TOTP can be used  
- 🆕 **Logging of failed attempts** with reasons  

Would you like any additional details or improvements? 🚀