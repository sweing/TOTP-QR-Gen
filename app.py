import json
import os
import datetime
from flask import Flask, request, jsonify, render_template
import pyotp
import base64
from dotenv import load_dotenv
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend

app = Flask(__name__)
load_dotenv()

# Load user secrets from .env (stored as a JSON-like string)
USER_SECRETS = json.loads(os.getenv("TOTP_SECRETS", "{}"))
MAX_VALIDATIONS = int(os.getenv("MAX_VALIDATIONS", "1"))  # Number of times a TOTP can be validated
validation_logs = []  # Store validation attempts (successful and failed)
validation_counts = {}  # Track validation count per TOTP

def decrypt_totp(secret_key: str, encrypted_totp: str) -> str:
    """Decrypts the TOTP code using AES-GCM with the secret key."""
    try:
        key = secret_key[:32].ljust(32, '0').encode()  # Ensure key is 32 bytes
        encrypted_data = base64.urlsafe_b64decode(encrypted_totp)

        iv = encrypted_data[:12]  # First 12 bytes = IV (Nonce)
        tag = encrypted_data[12:28]  # Next 16 bytes = Authentication tag
        ciphertext = encrypted_data[28:]  # Rest = Encrypted TOTP

        cipher = Cipher(algorithms.AES(key), modes.GCM(iv, tag), backend=default_backend())
        decryptor = cipher.decryptor()
        decrypted_totp = decryptor.update(ciphertext) + decryptor.finalize()

        return decrypted_totp.decode()
    except Exception as e:
        print(f"Decryption error: {e}")
        return None

@app.route('/validate', methods=['GET'])
def validate_totp():
    """Validates the encrypted TOTP from the QR code."""
    account_id = request.args.get('account_id')
    encrypted_totp = request.args.get('totp_enc')
    timestamp = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
    user_ip = request.remote_addr

    if not account_id or not encrypted_totp:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Missing parameters", "ip": user_ip})
        return jsonify({"success": False, "error": "Missing parameters"}), 400

    # Get the user's secret key
    secret = USER_SECRETS.get(account_id)
    if not secret:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Invalid account ID", "ip": user_ip})
        return jsonify({"success": False, "error": "Invalid account ID"}), 400

    # Decrypt the TOTP
    decrypted_totp = decrypt_totp(secret, encrypted_totp)
    if not decrypted_totp:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Invalid TOTP encryption", "ip": user_ip})
        return jsonify({"success": False, "error": "Invalid TOTP encryption"}), 400

    # Check if the TOTP has exceeded max validations
    if decrypted_totp in validation_counts and validation_counts[decrypted_totp] >= MAX_VALIDATIONS:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "TOTP validation limit reached", "ip": user_ip})
        return jsonify({"success": False, "error": "TOTP validation limit reached"}), 400

    # Validate the TOTP
    totp = pyotp.TOTP(secret)
    if totp.verify(decrypted_totp):
        validation_counts[decrypted_totp] = validation_counts.get(decrypted_totp, 0) + 1
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "success", "ip": user_ip})
        return jsonify({"success": True, "message": "TOTP is valid"}), 200
    else:
        validation_logs.append({"timestamp": timestamp, "account_id": account_id, "status": "failed", "reason": "Invalid TOTP", "ip": user_ip})
        return jsonify({"success": False, "error": "Invalid TOTP"}), 400

@app.route('/admin', methods=['GET'])
def admin_dashboard():
    """Admin dashboard to view validation logs."""
    return render_template('admin.html', validation_logs=validation_logs, max_validations=MAX_VALIDATIONS)

if __name__ == '__main__':
    app.run(debug=True)
